// Abstract class for accessing the common features of graphics APIs like DirectX and OpenGL
#pragma once

#include "VertApi.h"
#include "FileMgr.h"

struct GpuDrawable;
struct GpuSprite;

struct GpuRect{
	float left;
	float top;
	float right;
	float bottom;
	GpuRect(float l, float t, float r, float b)
		: left(l), top(t), right(r), bottom(b) {}
	GpuRect()
		: left(0.0f), top(0.0f), right(0.0f), bottom(0.0f) {}
};

struct GpuEventResponse
{
	virtual void Respond(){}
};

struct GpuFont {
	float colorR;
	float colorG;
	float colorB;
	float colorA;
	bool pixelSpace;
	virtual ~GpuFont() {};
protected:
	GpuFont() : colorR(0.0f), colorG(0.0f),
	colorB(0.0f), colorA(1.0f), pixelSpace(true) {}
};

enum GpuFontStyle
{
	GpuFontStyle_Regular,
	GpuFontStyle_Bold,
	GpuFontStyle_Italic
};

struct GpuTexture {
	virtual ~GpuTexture() {};
protected:
	GpuTexture(){}
};

struct GpuMesh {
	float positionX;
	float positionY;
	float positionZ;
	float rotationX;
	float rotationY;
	float rotationZ;
	float colorR;
	float colorG;
	float colorB;
	float colorA;
	GpuTexture * texture;
	bool wrapTexture;
	GpuMesh() :
		positionX(0.0f), positionY(0.0f), positionZ(0.0f),
		rotationX(0.0f), rotationY(0.0f), rotationZ(0.0f),
		colorR(1.0f), colorG(0.0f), colorB(0.0f), colorA(1.0f),
		texture(0), wrapTexture(false) {}
	void setPosition(float x, float y, float z)
	{
		positionX = x; positionY = y; positionZ = z;
	}
	void setColor(float r, float g, float b, float a)
	{
		colorR = r; colorG = g; colorB = b; colorA = a;
	}
	virtual ~GpuMesh()
	{
		//if(texture) delete texture;
	}
};

struct GpuIndexedMesh : public GpuMesh {
	GpuIndexedMesh() : GpuMesh() {}
	virtual ~GpuIndexedMesh(){}
};

enum GpuLightType
{
	GpuLightType_Directional = 0,
	GpuLightType_Point,
	GpuLightType_Spot
};

struct GpuLight{
	float diffuseR, diffuseG, diffuseB;
	GpuLight() : diffuseR(1.0f), diffuseG(1.0f), diffuseB(1.0f) {}
	void setColor(float r, float g, float b)
	{
		diffuseR = r; diffuseG = g; diffuseB = b;
	}
	virtual GpuLightType GetType() = 0;
};

struct GpuDirectionalLight : public GpuLight {
	float u, v, w;
	void SetDirection(float x, float y, float z)
	{
		u = x; v = y; w = z;
	}
	virtual GpuLightType GetType() override
	{
		return GpuLightType_Directional;
	}
};

struct GpuPointLight : public GpuLight {
	float x, y, z;
	void SetPosition(float x, float y, float z)
	{
		this->x = x; this->y = y; this->z = z;
	}
	virtual GpuLightType GetType() override
	{
		return GpuLightType_Point;
	}
};

struct GpuSpotLight : public GpuLight {
	float x, y, z;
	float u, v, w;
	float power;
	void SetPosition(float x, float y, float z)
	{
		this->x = x; this->y = y; this->z = z;
	}
	void SetDirection(float u, float v, float w)
	{
		this->u = u; this->v = v; this->w = w;
	}
	virtual GpuLightType GetType() override
	{
		return GpuLightType_Spot;
	}
};

struct GpuCamera{
	float x, y, z;
	float targetX, targetY, targetZ;
	float upX, upY, upZ;
};

struct GpuScene{
	GpuIndexedMesh** meshes;
	unsigned numMeshes;
	GpuLight** lights;
	unsigned numLights;
	GpuCamera* camera;
};

class GpuApi;

struct GpuDrawable
{
	virtual void Draw(GpuApi* gpu) = 0;
};

class GpuApi 
{
	int screenWidth;
	int screenHeight;

protected:
	bool initialised;

	enum GpuSpecialBuffer{
		GpuSpecialBuffer_None,
		GpuSpecialBuffer_BackBuffer,
		GpuSpecialBuffer_Stencil,
		GpuSpecialBuffer_StencilClip,
		GpuSpecialBuffer_StencilShadow
	};

	struct GpuDrawBuffer{
	private:
		GpuSpecialBuffer special;
	public:
		GpuDrawBuffer() : special(GpuSpecialBuffer_None) {}
		GpuDrawBuffer(GpuSpecialBuffer s) : special(s)   {}
		GpuSpecialBuffer GetSpecial() { return special; }
	};

public:
	const unsigned standardScreenHeight;

	GpuApi() : initialised(false), standardScreenHeight(768) {}
	virtual ~GpuApi() {};

	virtual void LoadShaders(FileMgr* fileSystem) = 0;

	virtual void Clear() = 0;
	virtual void BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void Present() = 0;

	virtual void Draw(GpuDrawable* drawable) { drawable->Draw(this); }
	virtual void DrawGpuText(GpuFont* font, const wchar_t* text, float x, float y, bool center) = 0;
	virtual void DrawGpuMesh(GpuMesh* mesh, GpuCamera* camera, GpuLight** lights, unsigned numLights, 
		bool wireFrame = false, GpuDrawBuffer* buffer = 0) = 0;
	virtual void DrawGpuIndexedMesh(GpuIndexedMesh* mesh, GpuCamera* camera, GpuLight** lights, 
		unsigned numLights, bool wireFrame = false, GpuDrawBuffer* buffer = 0) = 0;
	virtual void DrawGpuSprite(GpuSprite* sprite) = 0;
	virtual void DrawGpuScene(GpuScene* scene) = 0;

	virtual void LookTransform(
		float viewer_x, float viewer_y, float viewer_z,
		float target_x, float target_y, float target_z,
		float wayup_x, float wayup_y, float wayup_z) = 0;
	virtual void PerspectiveTransform(
		float fov_y, float aspectratio, 
		float z_near, float z_far) = 0;

	virtual GpuFont* CreateGpuFont(int height,const wchar_t* facename,
		GpuFontStyle style = GpuFontStyle_Regular) = 0;
	virtual GpuTexture* CreateGpuTextureFromFile(const wchar_t* path) = 0;
	virtual GpuMesh* CreateGpuMesh(
		unsigned numVertices, unsigned size, void* vertexBufferData, unsigned numTriangles) = 0;
	virtual GpuMesh* CreateGpuMesh(VertexBuffer* buffer, unsigned numTriangles) = 0;
	virtual GpuIndexedMesh* CreateGpuIndexedMesh(
		unsigned numVertices, unsigned vertexDataSize, void* vertexData,
		unsigned numTriangles, unsigned indexDatumSize, unsigned* indexData, VertexType type) = 0;
	virtual GpuIndexedMesh* CreateGpuIndexedMesh(VertexBuffer* buffer, unsigned numTriangles, 
		unsigned indexDataSize, unsigned* indexData) = 0;

	virtual GpuIndexedMesh* CreateTeapot() = 0;
	virtual GpuIndexedMesh* CreateCube() = 0;
	virtual GpuIndexedMesh* CreateCylinder(float radius, float length, unsigned slices, unsigned stacks, bool texCoords = false) = 0;
	virtual GpuIndexedMesh* CreateSphere(float radius, unsigned slices, unsigned stacks, bool texCoords = false) = 0;
	virtual GpuIndexedMesh* CreateGrid(float width, float depth, unsigned columns, unsigned rows, GpuRect* textureRect = 0) = 0;
	//virtual GpuIndexedMesh* ParseIndexedMesh(char* text) = 0;

	GpuDrawBuffer* GetStencilBuffer() { return new GpuDrawBuffer(GpuSpecialBuffer_Stencil); }
	GpuDrawBuffer* GetStencilClipBuffer() { return new GpuDrawBuffer(GpuSpecialBuffer_StencilClip); }
	
	virtual float MeasureGpuText(GpuFont* font, const wchar_t* text) = 0;
	virtual void SetClearColor(float r, float g, float b) = 0;
	virtual void SetScreenSize(int width, int height) = 0;
	int GetScreenWidth() { return screenWidth; }
	int GetScreenHeight() { return screenHeight; }
	unsigned GetStandardScreenHeight() { return standardScreenHeight; }
	bool isInitialised() { return initialised; }
	virtual GpuRect GetTextureDimensions(GpuTexture *texture) = 0;

	virtual bool isDeviceLost() = 0;

	virtual VertApi* GetVertApi() = 0;

	// - Check Device Capabilities
};

struct GpuSprite : public GpuDrawable
{
	GpuTexture* texture;
	float transformCenterX;
	float transformCenterY;
	float size;
	float positionX;
	float positionY;
	float positionZ;
	bool pixelSpace;
	bool brightAsAlpha;
	float rotation;
	float colorR;
	float colorG;
	float colorB;
	float colorA;
	float scaleX;
	float scaleY;
	GpuRect clipRect;
	
	GpuSprite(
			GpuTexture *texture = 0, 
			float transformCenterX = 0.0f, 
			float transformCenterY = 0.0f, 
			float size = 1.0f)
		: texture(texture), transformCenterX(transformCenterX), 
		transformCenterY(transformCenterY), size(size), 
		positionX(0.0f), positionY(0.0f), positionZ(0.0f), 
		pixelSpace(false), brightAsAlpha(false), rotation(0.0f), 
		colorR(1.0f), colorG(1.0f), colorB(1.0f), colorA(1.0f),
		scaleX(1.0f), scaleY(1.0f), clipRect(0.0f,0.0f,1.0f,1.0f) {}

	virtual void Draw(GpuApi* gpu) override
	{
		gpu->DrawGpuSprite(this);
	}
};

#pragma once

#include "FilesApi.h"
#include "AssetMgr.h"
#include "GpuShaders.h"
#include "GpuVertices.h"
#include "StepMgr.h"

class GpuApi;
struct GpuShaderLoader;

struct GpuDrawable
{
	virtual void BeDrawn(GpuApi * gpu, GpuDrawSurface * surface = 0) = 0;
};

/* Abstract class to interact with graphics APIs like DirectX and OpenGL */
class GpuApi 
{
protected:
	//bool initialised;
	bool drawEverythingWireframe;

	GpuDrawSurface * stencilSurface;
	GpuDrawSurface * stencilClipSurface;

public:
	const unsigned standardScreenHeight;

	GpuApi() : 
		//initialised(false), 
		drawEverythingWireframe(false),
		standardScreenHeight(768),
		stencilSurface(0),
		stencilClipSurface(0) {}
	virtual ~GpuApi() {};

	virtual void Initialize(AssetMgr * assets) = 0;
	virtual void OnCriticalLoad(AssetMgr * assets) = 0;
	virtual void Clear() = 0;
	virtual void BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void Present() = 0;

	virtual inline void Draw(GpuDrawable * drawable, GpuDrawSurface * surface = 0) { drawable->BeDrawn(this,surface); }

	virtual void DrawGpuSprite(GpuSprite * sprite, GpuDrawSurface * buffer = 0) = 0;
	virtual void DrawGpuText(GpuFont * font, const wchar_t* text, float x, float y, 
		bool center, GpuDrawSurface * surface = 0) = 0;
	virtual void DrawGpuModel(GpuModel * model, GpuCamera * camera, GpuLight ** lights, 
		unsigned numLights, GpuDrawSurface * surface = 0, GpuInstanceBuffer * instances = 0) = 0;
	virtual void DrawGpuSurface(GpuDrawSurface * source, GpuEffect * effect, GpuDrawSurface * dest) = 0;

	virtual GpuFont * CreateGpuFont(int height, const wchar_t * facename, GpuFontStyle style = GpuFontStyle_Regular) = 0;
	virtual GpuTexture * CreateGpuTexture(char * data, unsigned dataSize, bool isDDS = false) = 0;
	virtual GpuCubeMap * CreateGpuCubeMap(char * data, unsigned dataSize) = 0;
	virtual GpuVolumeTexture * CreateGpuVolumeTexture(char * data, unsigned dataSize) = 0;
	virtual GpuShaderLoader * CreateGpuShaderLoader(FileApi_Directory * directory, const wchar_t * path) = 0;

	virtual GpuMesh * CreateGpuMesh(unsigned numVertices, void * vertexData, VertexType type, bool dynamic = false) = 0;
	virtual GpuMesh * CreateGpuMesh(unsigned numVertices, void * vertexData,
		unsigned numTriangles, unsigned * indexData, VertexType type, bool dynamic = false) = 0;

	GpuMesh * CreateGpuMesh(IVertexBuffer * buffer, bool dynamic = false)
	{
		return CreateGpuMesh(
			buffer->GetLength(), 
			buffer->GetData(), 
			buffer->GetVertexType(),
			dynamic);
	}
	GpuMesh * CreateGpuMesh(IVertexBuffer * buffer, unsigned numTriangles, unsigned * indexData, bool dynamic = false)
	{
		return CreateGpuMesh(
			buffer->GetLength(), 
			buffer->GetData(),
			numTriangles,
			indexData,
			buffer->GetVertexType(),
			dynamic);
	}
	
	virtual GpuInstanceBuffer * CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type) = 0;

	virtual void UpdateDynamicMesh(GpuMesh * dynamicMesh, IVertexBuffer * buffer) = 0;
	virtual void UpdateDynamicMesh(GpuMesh * dynamicMesh, IVertexBuffer * buffer, unsigned numTriangles, unsigned * indexData) = 0;
	virtual void UpdateInstanceBuffer(GpuInstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData) = 0;

	GpuDrawSurface * GetStencilSurface() { return stencilSurface; }
	GpuDrawSurface * GetStencilClipSurface() { return stencilClipSurface; }
	virtual GpuDrawSurface * CreateDrawSurface(unsigned width, unsigned height) = 0;
	virtual GpuDrawSurface * CreateFullscreenDrawSurface() = 0;

	virtual void AddDeviceListener(IGpuDeviceListener * listener) = 0;
	virtual void RemoveDeviceListener(IGpuDeviceListener * listener) = 0;

	virtual float MeasureGpuText(GpuFont * font, const wchar_t * text) = 0;
	virtual void SetClearColor(float r, float g, float b, float a) = 0;

	// Should the GPU keep a reference to the window?
	virtual void OnScreenResize(unsigned width, unsigned height) = 0;
	virtual void GetBackbufferSize(unsigned & width, unsigned & height) = 0;
	virtual void SetMultisampling(unsigned multisampleLevel) = 0;

	unsigned GetStandardScreenHeight() { return standardScreenHeight; }
	//bool isInitialised() { return initialised; }
	virtual void SetDrawWireframe(bool wireframe) { drawEverythingWireframe = wireframe; }

	//virtual GpuRect GetTextureDimensions(GpuTexture * texture) = 0;

	virtual bool isDeviceLost() = 0;

	// - Check Device Capabilities
};

struct GpuSprite : public GpuDrawable
{
	GpuTexture * texture;
	glm::vec2 center;
	float size;
	glm::vec3 position;
	float rotation;
	glm::vec4 color;
	glm::vec2 scale;
	bool pixelSpace;
	bool brightAsAlpha;
	GpuRect clipRect;
	
	GpuSprite(
			GpuTexture * texture = 0, 
			float transformCenterX = 0.0f, 
			float transformCenterY = 0.0f, 
			float size = 1.0f)
		: texture(texture), center(transformCenterX,transformCenterY), 
		size(size), rotation(0.0f), color(1.0f,1.0f,1.0f,1.0f), 
		scale(1.0f,1.0f), pixelSpace(false), brightAsAlpha(false), 
		clipRect(0.0f,0.0f,1.0f,1.0f) {}

	virtual void BeDrawn(GpuApi* gpu, GpuDrawSurface * surface = 0) override
	{
		gpu->DrawGpuSprite(this, surface);
	}
};

struct GpuComplexModel : public IAsset
{
	GpuModel * models;
	unsigned numModels;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale; // NON-UNIFORM SCALING IS DANGEROUS, CAN UNNORMALIZE NORMALS!!!
	
	bool wireframe;

	GpuComplexModel(int numModels) :
		models(0),
		numModels(numModels),
		scale(1.0f,1.0f,1.0f),
		wireframe(false)
	{
		models = new GpuModel[numModels];
	}

	virtual ~GpuComplexModel()
	{
		delete[] models;
	}

	virtual AssetType GetType() override { return ModelAsset; }
	virtual IAsset * GetAsset() override 
	{
		GpuComplexModel * dst = new GpuComplexModel(numModels);
		for(unsigned i = 0; i < numModels; ++i)
		{
			dst->models[i] = models[i];
		}
		return dst;
	}

	virtual void BeDrawn(GpuApi * gpu, GpuCamera * camera, GpuLight ** lights, 
		unsigned numLights, GpuDrawSurface * buffer = 0)
	{
		GpuModel tempModel;

		for(unsigned i = 0; i < numModels; ++i)
		{
			tempModel = models[i];
			tempModel.position += position; // with respect to rotation about the parent model's axis??
			tempModel.rotation += rotation;
			tempModel.scale *= scale;
			tempModel.wireframe = wireframe ? true : tempModel.wireframe;
			tempModel.destructMesh = false;

			gpu->DrawGpuModel(&tempModel, camera, lights, numLights, buffer);
		}
	}
};

struct GpuTextureLoader : public SimpleLoader
{
	GpuApi * gpu;

	GpuTexture * texture;
	GpuCubeMap * cubeMap;
	GpuVolumeTexture * volumeTex;

	GpuTextureLoader(GpuApi * gpu, FileApi * files, FileApi_Directory * directory, 
		const wchar_t * path, AssetType type) :
		SimpleLoader(files, directory, path, type), gpu(gpu),
		texture(0), cubeMap(0), volumeTex(0) {}

	virtual void Respond() override
	{
		if(buffer)
		{
			bool isDDS = (path.rfind(L".dds") == path.length() - 4);

			switch(type)
			{
				case TextureAsset:
					texture = gpu->CreateGpuTexture(buffer, bufferLength, isDDS);
					break;
				case CubeMapAsset:
					cubeMap = gpu->CreateGpuCubeMap(buffer, bufferLength);
					break;
				case VolumeTexAsset:
					volumeTex = gpu->CreateGpuVolumeTexture(buffer, bufferLength);
					break;
			}
		}
	}

	GpuTexture * GetTexture() { return texture; }
	GpuCubeMap * GetCubeTex() { return cubeMap; }
	GpuVolumeTexture * GetVolumeTex() { return volumeTex; }

	virtual IAsset * GetAsset()
	{
		switch(type)
		{
			case TextureAsset:
				return texture;
			case CubeMapAsset:
				return cubeMap;
			case VolumeTexAsset:
				return volumeTex;
			default:
				return 0;
		}
	}
};

// Copies a LARGE local mesh to a GPU mesh over potentially several frames
struct GpuMeshStreamer : public Steppable
{

};

// Concrete class to interact with the DirectX 9 Framework
#pragma once

#if defined(DEBUG) | defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#if defined(USE_DX9_GPUAPI)

//#pragma comment(lib,"$(DXSDK_DIR)Lib\x86\d3d9.lib")
//#pragma comment(lib,"d3dx9.lib")

#if defined(DEBUG) | defined(_DEBUG)
#define COMPILED_SHADER_PATH L"BaseShader.cso"
#else
#define COMPILED_SHADER_PATH L"BaseShader.cso"
#endif

#include "DX9VertApi.h"
#include "GpuApi.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <list>

struct DX9_GpuFont : public GpuFont
{
	ID3DXFont* fontObject;
	DX9_GpuFont(ID3DXFont* object)
	: GpuFont(), fontObject(object) {}
	~DX9_GpuFont();
};

struct DX9_GpuTexture : public GpuTexture
{
	IDirect3DTexture9* textureObject;
	DX9_GpuTexture(IDirect3DTexture9* object) { textureObject = object; }
	~DX9_GpuTexture();
};

struct DX9_GpuMesh : public GpuMesh
{
	IDirect3DVertexBuffer9* vertexBuffer; 
	IDirect3DVertexDeclaration9* vertexDeclaration;
	unsigned vertexSize;
	unsigned numVertices;
	unsigned numTriangles;
	VertexType vertexType;
	~DX9_GpuMesh();
};

struct DX9_GpuIndexedMesh : public GpuIndexedMesh
{
	IDirect3DVertexBuffer9* vertexBuffer; 
	IDirect3DVertexDeclaration9* vertexDeclaration;
	IDirect3DIndexBuffer9* indexBuffer;
	unsigned vertexSize;
	unsigned numVertices;
	unsigned numTriangles;
	VertexType vertexType;
	~DX9_GpuIndexedMesh();
};

class DX9_GpuApi : public GpuApi
{
	IDirect3D9* direct3D;
	IDirect3DDevice9* direct3Ddevice;
	ID3DXSprite* spriteInterface;
	ID3DXEffect* baseShader;
	D3DXMATRIX view;
	D3DXMATRIX projection;

	D3DCOLOR clearColor;

	D3DPRESENT_PARAMETERS presentParameters;

	std::list<ID3DXFont*> deviceDependentFonts;

	void SetBufferState(GpuDrawBuffer* buffer);
	void RestoreBufferState(GpuDrawBuffer* buffer);

	void SetShaderFloat(const char* name, float value);
	void SetShaderVec3(const char* name, float x, float y, float z);
	void SetShaderColor(const char* name, float r, float g, float b, float a);

	void onLostDevice();
	void onResetDevice();

	DX9_GpuIndexedMesh* D3DXToGpuIndexedMesh(ID3DXMesh*);
	D3DCOLOR floatToD3DColor(float r, float g, float b, float a) const;

	DX9_VertApi vtx;

public:
	DX9_GpuApi(HWND windowHandle);
	~DX9_GpuApi();

	void LoadShaders(FileMgr*) override;

	void Clear() override;
	void BeginScene() override;
	void EndScene() override;
	void Present() override;
	
	void DrawGpuText(GpuFont* font, LPCWSTR text, float x, float y, bool center) override;
	void DrawGpuMesh(GpuMesh* mesh, GpuCamera* camera, GpuLight** lights, 
		unsigned numlights, bool wireFrame = false, GpuDrawBuffer* buffer = 0) override;
	void DrawGpuIndexedMesh(GpuIndexedMesh* mesh, GpuCamera* camera, GpuLight** lights, 
		unsigned numlights, bool wireFrame = false, GpuDrawBuffer* buffer = 0) override;
	void DrawGpuSprite(GpuSprite* sprite) override;
	void DrawGpuScene(GpuScene* scene) override;

	void LookTransform(float,float,float,float,float,float,float,float,float) override;
	void PerspectiveTransform(float,float,float,float) override;

	GpuFont* CreateGpuFont(int height, LPCWSTR facename, 
		GpuFontStyle style = GpuFontStyle_Regular) override;
	GpuTexture* CreateGpuTextureFromFile(LPCWSTR path) override;
	GpuMesh* CreateGpuMesh(
		unsigned numVertices, unsigned size, void* vertexBufferData, unsigned numTriangles) override;
	GpuMesh* CreateGpuMesh(VertexBuffer* buffer, unsigned numTriangles) override;
	GpuIndexedMesh* CreateGpuIndexedMesh(
		unsigned numVertices, unsigned vertexDataSize, void* vertexData,
		unsigned numTriangles, unsigned indexDataSize, unsigned* indexData, VertexType type) override;
	GpuIndexedMesh* CreateGpuIndexedMesh(VertexBuffer* buffer, unsigned numTriangles, 
		unsigned indexDataSize, unsigned* indexData) override;

	GpuIndexedMesh* CreateTeapot() override;
	GpuIndexedMesh* CreateCube() override;
	GpuIndexedMesh* CreateCylinder(float radius, float length, unsigned slices, unsigned stacks, bool texCoords = false) override;
	GpuIndexedMesh* CreateSphere(float radius, unsigned slices, unsigned stacks, bool texCoords = false) override;
	GpuIndexedMesh* CreateGrid(float width, float depth, unsigned columns, unsigned rows, GpuRect* textureRect = 0) override;

	virtual float MeasureGpuText(GpuFont* font, const wchar_t* text) override;
	void SetClearColor(float r, float g, float b) override;
	void SetScreenSize(int width, int height) override;
	virtual GpuRect GetTextureDimensions(GpuTexture *texture) override;

	bool isDeviceLost() override;

	virtual VertApi* GetVertApi() override { return &vtx; }
};

#endif
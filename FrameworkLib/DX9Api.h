#pragma once

#if defined(DEBUG) | defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#if defined(USE_DX9_GPUAPI)

//#pragma comment(lib,"$(DXSDK_DIR)Lib\x86\d3d9.lib")
//#pragma comment(lib,"d3dx9.lib")

#include "GpuApi.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <list>
#include <map>
#include <string>

class DX9_GpuApi;

struct DX9_GpuFont : public GpuFont, IGpuDeviceListener
{
	ID3DXFont * fontObject;
	DX9_GpuApi * dx9gpu;
	float size;

	DX9_GpuFont(DX9_GpuApi * gpu, ID3DXFont * object);
	virtual ~DX9_GpuFont();
	virtual void OnLostDevice(GpuApi * gpu) override { fontObject->OnLostDevice(); }
	virtual void OnResetDevice(GpuApi * gpu) override { fontObject->OnResetDevice(); }
};

struct DX9_GpuTexture : public GpuTexture
{
	IDirect3DTexture9 * textureObject;
	unsigned width;
	unsigned height;
	DX9_GpuTexture(IDirect3DTexture9* object, unsigned width, unsigned height) : 
		textureObject(object), width(width), height(height) {}
	virtual ~DX9_GpuTexture() { if(textureObject) textureObject->Release(); }
	virtual unsigned GetWidth() override { return width; }
	virtual unsigned GetHeight() override { return height; }
};

struct DX9_GpuCubeMap : public GpuCubeMap
{
	IDirect3DCubeTexture9 * cubeMapObject;
	DX9_GpuCubeMap(IDirect3DCubeTexture9* object) : GpuCubeMap(), cubeMapObject(object) {}
	virtual ~DX9_GpuCubeMap() { if(cubeMapObject) cubeMapObject->Release(); }
};

struct DX9_GpuVolumeTexture : public GpuVolumeTexture
{
	IDirect3DVolumeTexture9 * textureObject;
	DX9_GpuVolumeTexture(IDirect3DVolumeTexture9 * object) :
		GpuVolumeTexture(), textureObject(object) {}
	virtual ~DX9_GpuVolumeTexture () { if(textureObject) textureObject->Release(); }
};

struct DX9_GpuMesh : public GpuMesh, IGpuDeviceListener
{
	IDirect3DVertexBuffer9 * vertexBuffer; 
	//IDirect3DVertexDeclaration9 * vertexDeclaration;
	IDirect3DIndexBuffer9 * indexBuffer;
	unsigned vertexSize;
	unsigned numVertices;
	unsigned numTriangles;
	VertexType vertexType;

	DX9_GpuApi * dx9gpu;
	
	char * localVertexData;
	unsigned * localIndexData;
	bool dynamic;

	DX9_GpuMesh(DX9_GpuApi * gpu): 
		vertexBuffer(0), 
		//vertexDeclaration(0),
		indexBuffer(0),
		vertexSize(0), 
		numVertices(0),
		numTriangles(0), 
		vertexType(VertexType_PosCol),
		dx9gpu(gpu),
		localVertexData(0),
		localIndexData(0),
		dynamic(false) {}
	virtual bool IsIndexed() { return indexBuffer != 0; }
	virtual bool IsDynamic() { return dynamic; }
	virtual ~DX9_GpuMesh();
	virtual void OnLostDevice(GpuApi * gpu) override
	{
		if(dynamic)
		{
			vertexBuffer->Release();
			if(indexBuffer) indexBuffer->Release();
		}
	}
	virtual void OnResetDevice(GpuApi * gpu) override
	{
		if(dynamic)
		{

		}
	}
};

namespace tinyxml2
{
	class XMLElement;
}

class ShaderParser;

struct DX9_GpuInstanceBuffer : public GpuInstanceBuffer, IGpuDeviceListener
{
	IDirect3DVertexBuffer9 * buffer;
	void * localBuffer;
	unsigned instanceSize;
	unsigned numInstances;
	unsigned instanceCapacity;

	DX9_GpuApi * dx9gpu;

	DX9_GpuInstanceBuffer(DX9_GpuApi * gpu):
		GpuInstanceBuffer(type),
		buffer(0),
		localBuffer(0),
		instanceSize(0),
		numInstances(0),
		instanceCapacity(0),
		dx9gpu(gpu) {}
	virtual ~DX9_GpuInstanceBuffer();

	virtual unsigned GetLength() override { return numInstances; }
	virtual unsigned GetCapacity() override { return instanceCapacity; }
	virtual void OnLostDevice(GpuApi * gpu) override;
	virtual void OnResetDevice(GpuApi * gpu) override;
};

struct DX9_GpuShader;
struct DX9_ModelShader;
struct DX9_TextureShader;
struct DX9_GpuDrawSurface;

// Concrete class to interact with the DirectX 9 Framework
class DX9_GpuApi : public GpuApi
{
	struct ShaderLoader;

	IDirect3D9* direct3D;
	IDirect3DDevice9* direct3Ddevice;
	ID3DXSprite* spriteInterface;
	D3DVERTEXELEMENT9 * vertexElements[VertexType_Count];
	D3DVERTEXELEMENT9 * instanceElements[InstanceType_Count];
	IDirect3DVertexDeclaration9 * texShaderDeclaration;

	D3DXMATRIX view;
	D3DXMATRIX projection;

	D3DCOLOR clearColor;
	D3DPRESENT_PARAMETERS presentParameters;

	void OnLostDevice();
	void OnResetDevice();

	DX9_GpuMesh * D3DXToGpuMesh(ID3DXMesh*);
	void CreateVertexBuffer(IDirect3DVertexBuffer9 ** output, unsigned dataSize, bool dynamic);
	void FillVertexBuffer(IDirect3DVertexBuffer9 ** target, void * input, unsigned dataSize, bool dynamic);
	void CreateIndexBuffer(IDirect3DIndexBuffer9 ** output, unsigned dataSize, bool dynamic);
	void FillIndexBuffer(IDirect3DIndexBuffer9 ** target, void * input, unsigned dataSize, bool dynamic);
	D3DCOLOR floatToD3DColor(float r, float g, float b, float a) const;
	void ApplySamplerParameters(GpuEffect * effect);
	void SetupVertexInformation();
	
	FileApi * files;
	StepMgr * steppables;

	DX9_ModelShader * baseShader;
	DX9_TextureShader * texCopyShader;
	DX9_GpuMesh * texShaderQuad;

	bool renderStateWireframe;

public:
	std::list<IGpuDeviceListener*> deviceListeners;

	DX9_GpuApi(FileApi * files, StepMgr * steppables, HWND windowHandle);
	~DX9_GpuApi();

	void Clear() override;
	void BeginScene() override;
	void EndScene() override;
	void Present() override;
	
	virtual void Initialize(AssetMgr * assets) override;
	virtual void OnCriticalLoad(AssetMgr * assets) override;
	virtual void DrawGpuSprite(GpuSprite * sprite, GpuDrawSurface * surface = 0) override;
	virtual void DrawGpuText(GpuFont * font, LPCWSTR text, float x, float y, 
		bool center, GpuDrawSurface * surface = 0) override;
	virtual void DrawGpuModel(GpuModel * model, GpuCamera * camera, GpuLight ** lights, 
		unsigned numlights, GpuDrawSurface* surface = 0, GpuInstanceBuffer * instances = 0) override;
	virtual void DrawGpuSurface(GpuDrawSurface * source, GpuEffect * effect, GpuDrawSurface * dest) override;

	virtual GpuFont * CreateGpuFont(int height, LPCWSTR facename, 
		GpuFontStyle style = GpuFontStyle_Regular) override;
	virtual GpuTexture * CreateGpuTexture(char * data, unsigned dataSize, bool isDDS = false) override;
	virtual GpuCubeMap * CreateGpuCubeMap(char * data, unsigned dataSize) override;
	virtual GpuVolumeTexture * CreateGpuVolumeTexture(char * data, unsigned dataSize) override;
	virtual GpuShaderLoader * CreateGpuShaderLoader(FileApi_Directory * directory, const wchar_t * path) override;
	
	virtual GpuMesh * CreateGpuMesh(unsigned numVertices, void * vertexData, VertexType type, bool dynamic = false) override;
	virtual GpuMesh * CreateGpuMesh(unsigned numVertices, void * vertexData, unsigned numTriangles, unsigned * indexData, 
		VertexType type, bool dynamic = false) override;
	virtual GpuInstanceBuffer * CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type) override;

	virtual GpuMeshCopy * CreateLargeGpuMesh(LocalMesh * localMesh); // override;

	virtual void UpdateDynamicMesh(GpuMesh * mesh, IVertexBuffer * buffer) override;
	virtual void UpdateDynamicMesh(GpuMesh * mesh, IVertexBuffer * buffer, unsigned numTriangles, unsigned * indexData) override;
	virtual void UpdateInstanceBuffer(GpuInstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData) override;

	virtual GpuDrawSurface * CreateDrawSurface(unsigned width, unsigned height) override;
	virtual GpuDrawSurface * CreateFullscreenDrawSurface() override;

	virtual void AddDeviceListener(IGpuDeviceListener * listener) override { if(listener) deviceListeners.push_back(listener); }
	virtual void RemoveDeviceListener(IGpuDeviceListener * listener) override { if(listener) deviceListeners.remove(listener); }

	virtual float MeasureGpuText(GpuFont * font, const wchar_t * text) override;

	virtual void SetClearColor(float r, float g, float b, float a) override;
	virtual void OnScreenResize(unsigned width, unsigned height) override;
	virtual void GetBackbufferSize(unsigned & width, unsigned & height) override;
	virtual void SetMultisampling(unsigned multisampleLevel) override;

	void RecreateMesh(DX9_GpuMesh * mesh);
	void RecreateInstanceBuffer(DX9_GpuInstanceBuffer * instanceBuffer);

	bool isDeviceLost() override;
};

#endif
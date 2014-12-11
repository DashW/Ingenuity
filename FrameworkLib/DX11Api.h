#pragma once

#ifdef USE_DX11_GPUAPI

#include "GpuApi.h"
#include "DX11Samplers.h"
#include "DX11Shaders.h"

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
#include <d3d11.h>
#else
#include <d3d11_1.h>
#endif

#include <DirectXMath.h>
#include <list>
#include <string>
#include <map>

#pragma comment(lib,"d3d11.lib")
//#pragma comment(lib,"..//FW1FontWrapper_1_1//x86//FW1FontWrapper.lib")

namespace DirectX {

class CommonStates;
class SpriteBatch;
class SpriteFont;

}

namespace tinyxml2 {

class XMLElement;

}

namespace Ingenuity {

class AssetMgr;

namespace DX11 {

struct Font : public Gpu::Font
{
	DirectX::SpriteFont * fontObject;
	LPCWSTR fontFamily;
	float fontSize;

	Font(DirectX::SpriteFont * object, LPCWSTR family, float size)
		: fontObject(object), fontFamily(family), fontSize(size) {}
	virtual ~Font();
};

struct Texture : public Gpu::Texture
{
	ID3D11Texture2D * texture;
	ID3D11ShaderResourceView * shaderView;
	D3D11_TEXTURE2D_DESC desc;

	Texture(ID3D11Texture2D * texture, ID3D11ShaderResourceView * shaderView, D3D11_TEXTURE2D_DESC desc)
		: texture(texture), shaderView(shaderView), desc(desc) {}
	virtual ~Texture()
	{
		texture->Release();
		shaderView->Release();
	}
	virtual unsigned GetWidth()  override { return desc.Width; }
	virtual unsigned GetHeight() override { return desc.Height; }
};

struct CubeMap : public Gpu::CubeMap
{
	ID3D11Texture2D * cubeMap;
	ID3D11ShaderResourceView * shaderView;
	D3D11_TEXTURE2D_DESC desc;

	CubeMap(ID3D11Texture2D * cubeMap, ID3D11ShaderResourceView * shaderView, D3D11_TEXTURE2D_DESC desc)
		: cubeMap(cubeMap), shaderView(shaderView), desc(desc) {}
	virtual ~CubeMap()
	{
		cubeMap->Release();
		shaderView->Release();
	}
};

struct VolumeTexture : public Gpu::VolumeTexture
{
	ID3D11Texture3D * volume;
	ID3D11ShaderResourceView * shaderView;
	D3D11_TEXTURE3D_DESC desc;

	VolumeTexture(ID3D11Texture3D * volume, ID3D11ShaderResourceView * shaderView, D3D11_TEXTURE3D_DESC desc)
		: volume(volume), shaderView(shaderView), desc(desc) {}
	virtual ~VolumeTexture()
	{
		volume->Release();
		shaderView->Release();
	}
	unsigned GetWidth() { return desc.Width; }
	unsigned GetHeight() { return desc.Height; }
	unsigned GetDepth() { return desc.Depth; }
};

struct Mesh : public Gpu::Mesh
{
	ID3D11Buffer * vertexBuffer;
	ID3D11Buffer * indexBuffer;

	unsigned vertexSize;
	unsigned numVertices;
	unsigned numTriangles;
	VertexType vertexType;

	bool dynamic;

	Mesh() :
		vertexBuffer(0),
		indexBuffer(0),
		dynamic(false) {}
	virtual bool IsIndexed() { return indexBuffer != 0; }
	virtual ~Mesh() {
		if(vertexBuffer) vertexBuffer->Release(); vertexBuffer = 0;
		if(indexBuffer) indexBuffer->Release(); indexBuffer = 0;
	};
};

struct InstanceBuffer : public Gpu::InstanceBuffer
{
	ID3D11Buffer * buffer;
	unsigned instanceSize;
	unsigned numInstances;
	unsigned instanceCapacity;

	InstanceBuffer(ID3D11Device * device, InstanceType type, unsigned numInstances, void * data) :
		Gpu::InstanceBuffer(type),
		buffer(0),
		instanceSize(VertApi::GetInstanceSize(type)),
		numInstances(numInstances),
		instanceCapacity(numInstances)
	{
		unsigned instanceDataSize = numInstances * instanceSize;

		CD3D11_BUFFER_DESC bufferDesc(instanceDataSize, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		D3D11_SUBRESOURCE_DATA bufferData = { 0 };
		bufferData.pSysMem = data;
		device->CreateBuffer(&bufferDesc, data ? &bufferData : 0, &buffer);
	}
	virtual ~InstanceBuffer()
	{
		if(buffer)
		{
			buffer->Release();
		}
	}

	virtual unsigned GetLength() override { return numInstances; }
	virtual unsigned GetCapacity() override { return instanceCapacity; }
};

class ShaderParser;

class Api : public Gpu::Api
{
	friend class ShaderLoader;

	D3D_FEATURE_LEVEL featureLevel;

	ID3D11Device * direct3Ddevice;

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
	IDXGISwapChain* dxgiSwapChain;
#else
	IDXGISwapChain1* dxgiSwapChain;
#endif

	ID3D11DeviceContext * direct3Dcontext;
	ID3D11RenderTargetView * renderTargetView;
	ID3D11DepthStencilView * depthStencilView;
	ID3D11DepthStencilState * depthStencilState;
	ID3D11Texture2D * depthStencilBuffer;

	ID3D11RasterizerState * wireframeState;
	ID3D11RasterizerState * defaultRasterState;

	ID3D11DepthStencilState * stencilState;
	ID3D11DepthStencilState * stencilClipState;

	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	DirectX::SpriteBatch * spriteBatch;
	DirectX::CommonStates * commonStates;

	ModelShader * baseShader;
	TextureShader * texCopyShader;
	Mesh * texShaderQuad;

	unsigned backBufferWidth;
	unsigned backBufferHeight;

	void SetupVertexInformation();

	const D3D11_INPUT_ELEMENT_DESC * vertexDescs[VertexType_Count];
	unsigned vertexDescSizes[VertexType_Count];
	const D3D11_INPUT_ELEMENT_DESC * instanceDescs[InstanceType_Count];
	unsigned instanceDescSizes[InstanceType_Count];
	ID3D11InputLayout * texShaderLayout;
	ID3D11VertexShader * texVertexShader;
	bool texVertexShaderRequested;

	Files::Api * files;

	std::list<Gpu::IDeviceListener*> deviceListeners;

	static const unsigned QUERY_LATENCY = 5;
	struct ProfileData
	{
		ID3D11Query * disjointQuery[QUERY_LATENCY];
		ID3D11Query * timestampStartQuery[QUERY_LATENCY];
		ID3D11Query * timestampEndQuery[QUERY_LATENCY];
		bool queryStarted;
		bool queryFinished;
		unsigned drawCalls;
		unsigned stateChanges;

		ProfileData() : queryStarted(false), queryFinished(false), drawCalls(0), stateChanges(0)
		{
			for(unsigned i = 0; i < QUERY_LATENCY; ++i)
			{
				disjointQuery[i] = 0;
				timestampStartQuery[i] = 0;
				timestampEndQuery[i] = 0;
			}
		}
		~ProfileData()
		{
			for(unsigned i = 0; i < QUERY_LATENCY; ++i)
			{
				if(disjointQuery[i]) disjointQuery[i]->Release();
				if(timestampStartQuery[i]) timestampStartQuery[i]->Release();
				if(timestampEndQuery[i]) timestampEndQuery[i]->Release();
			}
		}
	};

	typedef std::map<std::wstring, ProfileData> ProfileMap;
	typedef std::list<ProfileData*> ProfileList;
	ProfileMap profiles;
	ProfileList activeProfiles;
	UINT64 currFrame;

	std::map<std::wstring, float> profileTimes;

	SamplerMgr * samplerMgr;

public:
	float clearColor[4];

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
	Api(Files::Api * files, HWND handle);
#else
	Api(Files::Api * files, Windows::UI::Core::CoreWindow ^ window);
#endif
	~Api();

	virtual void Initialize(AssetMgr * assets) override;
	virtual void OnCriticalLoad(AssetMgr * assets) override;
	virtual void Clear() override;
	virtual void BeginScene() override;
	virtual void EndScene() override;
	virtual void Present() override;

	virtual void DrawGpuSprite(Gpu::Sprite * sprite, Gpu::DrawSurface * surface = 0) override;
	virtual void DrawGpuText(Gpu::Font * font, LPCWSTR text, float x, float y, bool center, Gpu::DrawSurface * surface = 0) override;
	virtual void DrawGpuModel(Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights,
		unsigned numLights, Gpu::DrawSurface * surface = 0, Gpu::InstanceBuffer * instances = 0, Gpu::Effect * overrideEffect = 0) override;
	virtual void DrawGpuSurface(Gpu::DrawSurface * source, Gpu::Effect * effect, Gpu::DrawSurface * dest) override;

	virtual Gpu::Font * CreateGpuFont(int height, LPCWSTR facename, Gpu::FontStyle style = Gpu::FontStyle_Regular) override;
	virtual Gpu::Texture * CreateGpuTexture(char * data, unsigned dataSize, bool isDDS = false) override;
	virtual Gpu::CubeMap * CreateGpuCubeMap(char * data, unsigned dataSize) override;
	virtual Gpu::VolumeTexture * CreateGpuVolumeTexture(char * data, unsigned dataSize) override;
	virtual Gpu::ShaderLoader * CreateGpuShaderLoader(Files::Directory * directory, const wchar_t * path) override;

	virtual Gpu::Mesh * CreateGpuMesh(unsigned numVertices, void* vertexData, VertexType type, bool dynamic = false) override;
	virtual Gpu::Mesh * CreateGpuMesh(unsigned numVertices, void* vertexData, unsigned numTriangles, unsigned* indexData, VertexType type, bool dynamic = false) override;
	virtual Gpu::InstanceBuffer * CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type) override;
	virtual Gpu::DrawSurface * CreateDrawSurface(unsigned width, unsigned height, Gpu::DrawSurface::Format format = Gpu::DrawSurface::Format_4x8int) override;
	virtual Gpu::DrawSurface * CreateScreenDrawSurface(float widthFactor = 1.0f, float heightFactor = 1.0f, Gpu::DrawSurface::Format format = Gpu::DrawSurface::Format_4x8int) override;

	virtual void UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, IVertexBuffer * buffer) override;
	virtual void UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, unsigned numTriangles, unsigned * indexData) override;
	virtual void UpdateInstanceBuffer(Gpu::InstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData) override;

	virtual void AddDeviceListener(Gpu::IDeviceListener * listener) override { if(listener) deviceListeners.push_back(listener); }
	virtual void RemoveDeviceListener(Gpu::IDeviceListener * listener) override { if(listener) deviceListeners.remove(listener); }

	virtual void BeginTimestamp(const std::wstring name) override;
	virtual void EndTimestamp(const std::wstring name) override;
	virtual Gpu::TimestampData GetTimestampData(const std::wstring name) override;

	virtual float MeasureGpuText(Gpu::Font * font, const wchar_t * text) override;

	virtual void SetClearColor(float r, float g, float b, float a) override;
	virtual void OnScreenResize(unsigned width, unsigned height) override;
	virtual void GetBackbufferSize(unsigned & width, unsigned & height) override;
	virtual void SetMultisampling(unsigned multisampleLevel) override;
	virtual void SetAnisotropy(unsigned anisotropy) override { samplerMgr->SetAnisotropy(direct3Dcontext, anisotropy); }
	virtual void SetBlendMode(Gpu::BlendMode blendMode) override;
	virtual bool isDeviceLost() override;

	void ResetRenderTargets();
};

} // namespace DX11
} // namespace Ingenuity

#endif
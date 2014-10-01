#pragma once

#ifdef USE_GL_GPUAPI

#include "GpuApi.h"

#define _X86_
#include <windef.h>


namespace Ingenuity {

namespace Files {
	class Api;
}

namespace GL {

struct ModelShader;
struct TextureShader;

struct Mesh : public Gpu::Mesh
{
	unsigned vertexArrayObject;
	unsigned vertexBufferObject;

	unsigned indexBufferObject;

	//unsigned vertexSize;
	unsigned numVertices;
	unsigned numTriangles;
	VertexType vertexType;

	bool dynamic;

	Mesh() :
		vertexArrayObject(0),
		vertexBufferObject(0),
		indexBufferObject(0),
		//vertexSize(0),
		numVertices(0),
		numTriangles(0),
		vertexType(VertexType_Count),
		dynamic(false) {}
	virtual ~Mesh();

	virtual bool IsIndexed() { return indexBufferObject != 0; }
};

class Api : public Gpu::Api
{
	HDC deviceContext;
	HGLRC glContext;

	Files::Api * files;

	ModelShader * baseShader;
	TextureShader * texCopyShader;
	Mesh * texShaderQuad;

	unsigned backbufferWidth, backbufferHeight;

	static void APIENTRY DebugMessageCallback(unsigned source, unsigned type, unsigned id,
		unsigned severity, int length, const char* message, const void* userParam);

public:
	Api(Files::Api * files, HWND handle);
	virtual ~Api();

	unsigned texVertexShader;
	bool texVertexShaderRequested;

	virtual void Initialize(AssetMgr * assets) override;
	virtual void OnCriticalLoad(AssetMgr * assets) override;
	virtual void Clear() override;
	virtual void BeginScene() override;
	virtual void EndScene() override;
	virtual void Present() override;

	virtual void DrawGpuSprite(Gpu::Sprite * sprite, Gpu::DrawSurface * buffer = 0) override;
	virtual void DrawGpuText(Gpu::Font * font, const wchar_t* text, float x, float y,
		bool center, Gpu::DrawSurface * surface = 0) override;
	virtual void DrawGpuModel(Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights,
		unsigned numLights, Gpu::DrawSurface * surface = 0, Gpu::InstanceBuffer * instances = 0) override;
	virtual void DrawGpuSurface(Gpu::DrawSurface * source, Gpu::Effect * effect, Gpu::DrawSurface * dest) override;

	virtual Gpu::Font * CreateGpuFont(int height, const wchar_t * facename, Gpu::FontStyle style = Gpu::FontStyle_Regular) override;
	virtual Gpu::Texture * CreateGpuTexture(char * data, unsigned dataSize, bool isDDS = false) override;
	virtual Gpu::CubeMap * CreateGpuCubeMap(char * data, unsigned dataSize) override;
	virtual Gpu::VolumeTexture * CreateGpuVolumeTexture(char * data, unsigned dataSize) override;
	virtual Gpu::ShaderLoader * CreateGpuShaderLoader(Files::Directory * directory, const wchar_t * path) override;

	virtual Gpu::Mesh * CreateGpuMesh(unsigned numVertices, void * vertexData, VertexType type, bool dynamic = false) override;
	virtual Gpu::Mesh * CreateGpuMesh(unsigned numVertices, void * vertexData,
		unsigned numTriangles, unsigned * indexData, VertexType type, bool dynamic = false) override;

	virtual Gpu::InstanceBuffer * CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type) override;

	virtual void UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, IVertexBuffer * buffer) override;
	virtual void UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, unsigned numTriangles, unsigned * indexData) override;
	virtual void UpdateInstanceBuffer(Gpu::InstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData) override;

	virtual Gpu::DrawSurface * CreateDrawSurface(unsigned width, unsigned height, Gpu::DrawSurface::Format format = Gpu::DrawSurface::Format_4x8int) override;
	virtual Gpu::DrawSurface * CreateScreenDrawSurface(float widthFactor = 1.0f, float heightFactor = 1.0f, Gpu::DrawSurface::Format format = Gpu::DrawSurface::Format_4x8int) override;

	virtual void AddDeviceListener(Gpu::IDeviceListener * listener) override;
	virtual void RemoveDeviceListener(Gpu::IDeviceListener * listener) override;

	virtual void BeginTimestamp(const std::wstring name) override;
	virtual void EndTimestamp(const std::wstring name) override;
	virtual Gpu::TimestampData GetTimestampData(const std::wstring name) override;

	virtual float MeasureGpuText(Gpu::Font * font, const wchar_t * text) override;
	virtual void SetClearColor(float r, float g, float b, float a) override;

	virtual void OnScreenResize(unsigned width, unsigned height) override;
	virtual void GetBackbufferSize(unsigned & width, unsigned & height) override;
	virtual void SetMultisampling(unsigned multisampleLevel) override;
	virtual void SetBlendMode(Gpu::BlendMode blendMode) override;

	virtual void SetAnisotropy(unsigned anisotropy) override;

	virtual bool isDeviceLost() override;
};

} // namespace GL

} // namespace Ingenuity

#endif // USE_GL_GPUAPI
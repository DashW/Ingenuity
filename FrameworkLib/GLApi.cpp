#ifdef USE_GL_GPUAPI

#include "GLApi.h"
#include "GLShaders.h"

#include <Windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB    0x9126
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

namespace Ingenuity {
namespace GL {

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &vertexArrayObject);
	glDeleteBuffers(1, &vertexBufferObject);
	if(indexBufferObject)
	{
		glDeleteBuffers(1, &indexBufferObject);
	}
}

Api::Api(Files::Api * files, HWND handle) :
	deviceContext(0),
	glContext(0),
	files(files),
	baseShader(0),
	texCopyShader(0),
	texShaderQuad(0),
	backbufferWidth(0),
	backbufferHeight(0),
	texVertexShader(0),
	texVertexShaderRequested(false)
{
	deviceContext = GetDC(handle);

	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW; // Enable double buffering, opengl support and drawing to a window
	pfd.iPixelType = PFD_TYPE_RGBA; // Set our application to use RGBA pixels
	pfd.cColorBits = 32; // Give us 32 bits of color information
	pfd.cDepthBits = 32; // Give us 32 bits of depth information
	pfd.iLayerType = PFD_MAIN_PLANE; // Set the layer of the PFD

	int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
	
	SetPixelFormat(deviceContext, pixelFormat, &pfd);

	HGLRC tempContext = wglCreateContext(deviceContext);
	wglMakeCurrent(deviceContext, tempContext);
	
	GLenum error = glewInit();

	int attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3, // Set the MAJOR version of OpenGL to 3
		WGL_CONTEXT_MINOR_VERSION_ARB, 2, // Set the MINOR version of OpenGL to 2
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // Set our OpenGL context to be forward compatible
		0
	};

	if(wglewIsSupported("WGL_ARB_create_context") == 1) {
		glContext = wglCreateContextAttribsARB(deviceContext, 0, attributes);
		wglMakeCurrent(0, 0);
		wglDeleteContext(tempContext);
		wglMakeCurrent(deviceContext, glContext);
	}
	else
	{
		glContext = tempContext;
	}

	SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	glFrontFace(GL_CW);

	glEnable(GL_DEBUG_OUTPUT);

	glDebugMessageCallback(Api::DebugMessageCallback, 0);

	RECT windowRect;
	GetClientRect(handle, &windowRect);
	OnScreenResize(windowRect.right, windowRect.bottom);
}
Api::~Api()
{

}

void Api::DebugMessageCallback(unsigned source, unsigned type, unsigned id,
	unsigned severity, int length, const char* message, const void* userParam)
{
	std::string shortMessage = message;
	std::wstring wideMessage(shortMessage.begin(), shortMessage.end());

	OutputDebugString(wideMessage.c_str());
	OutputDebugString(L"\n");
}

void Api::Initialize(AssetMgr * assets)
{
	// Load the default shaders

	Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);

	assets->Load(frameworkDir, L"BaseShader.xml", ShaderAsset, 0, AssetMgr::CRITICAL_TICKET);
	//assets->Load(frameworkDir, L"TextureCopy.xml", ShaderAsset, 0, AssetMgr::CRITICAL_TICKET);
}

void Api::OnCriticalLoad(AssetMgr * assets)
{
	Files::Directory * frameworkDir = files->GetKnownDirectory(Files::FrameworkDir);

	Gpu::Shader * gpuBaseShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"BaseShader.xml");
	//Gpu::Shader * gpuTexCopyShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"TextureCopy.xml");

	baseShader = static_cast<GL::ModelShader*>(gpuBaseShader);
	//texCopyShader = static_cast<GL::TextureShader*>(gpuTexCopyShader);
}

void Api::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Api::BeginScene()
{
	glViewport(0, 0, backbufferWidth, backbufferHeight);
}

void Api::EndScene()
{

}

void Api::Present()
{
	SwapBuffers(deviceContext);
}

void Api::DrawGpuSprite(Gpu::Sprite * sprite, Gpu::DrawSurface * buffer)
{

}

void Api::DrawGpuText(Gpu::Font * font, const wchar_t* text, float x, float y,
	bool center, Gpu::DrawSurface * surface)
{

}

void Api::DrawGpuModel(Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights,
	unsigned numLights, Gpu::DrawSurface * surface, Gpu::InstanceBuffer * instances)
{
	if(!model) return;
	if(!model->mesh) return;
	if(model->backFaceCull && model->frontFaceCull) return;
	if(!baseShader) return;

	GL::Mesh * glMesh = static_cast<GL::Mesh*>(model->mesh);

	bool wireframe = model->wireframe || drawEverythingWireframe;
	glPolygonMode(GL_FRONT_AND_BACK , wireframe ? GL_LINE : GL_FILL);

	if(model->backFaceCull || model->frontFaceCull)
	{
		glEnable(GL_CULL_FACE);
		if(model->backFaceCull) glCullFace(GL_BACK);
		else if(model->frontFaceCull) glCullFace(GL_FRONT);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	VertexType vertexType = glMesh->vertexType;
	InstanceType instanceType = InstanceType_None;

	GL::ModelShader * shader = baseShader;
	if(model->effect && model->effect->shader && model->effect->shader->IsModelShader())
	{
		shader = static_cast<GL::ModelShader*>(model->effect->shader);
	}

	if(!shader->SetTechnique(vertexType, instanceType)) return;

	float aspect = float(backbufferWidth) / float(backbufferHeight);

	//GL::DrawSurface * dx11surface = static_cast<DX11::DrawSurface*>(surface);
	//if(dx11surface && dx11surface->GetSurfaceType() == Gpu::DrawSurface::TypeTexture)
	//{
	//	Gpu::Texture * surfaceTex = dx11surface->GetTexture();
	//	aspect = float(surfaceTex->GetWidth()) / float(surfaceTex->GetHeight());
	//}

	if(!shader->SetParameters(model, camera, lights, numLights, aspect)) return;

	//samplerMgr->ApplySamplerParams(
	//	direct3Dcontext,
	//	shader->currentTechnique->paramMappings,
	//	model->effect ? &model->effect->samplerParams : 0,
	//	true);

	glBindVertexArray(glMesh->vertexArrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh->indexBufferObject);

	//if(dx11surface) dx11surface->Begin();
	if(glMesh->IsIndexed())
	{
		//if(instances)
		//{
		//	direct3Dcontext->DrawIndexedInstanced(dx11mesh->numTriangles * 3, instances->GetLength(), 0, 0, 0);
		//}
		//else
		//{
			glDrawElements(GL_TRIANGLES, glMesh->numTriangles * 3, GL_UNSIGNED_INT, 0);
		//}
	}
	else
	{
		//if(instances)
		//{
		//	direct3Dcontext->DrawInstanced(dx11mesh->numVertices, instances->GetLength(), 0, 0);
		//}
		//else
		//{
			glDrawArrays(GL_TRIANGLES, 0, glMesh->numVertices);
		//}
	}
	//if(dx11surface) dx11surface->End();

}

void Api::DrawGpuSurface(Gpu::DrawSurface * source, Gpu::Effect * effect, Gpu::DrawSurface * dest)
{

}

Gpu::Font * Api::CreateGpuFont(int height, const wchar_t * facename, Gpu::FontStyle style)
{
	return 0;
}

Gpu::Texture * Api::CreateGpuTexture(char * data, unsigned dataSize, bool isDDS)
{
	return 0;
}

Gpu::CubeMap * Api::CreateGpuCubeMap(char * data, unsigned dataSize)
{
	return 0;
}

Gpu::VolumeTexture * Api::CreateGpuVolumeTexture(char * data, unsigned dataSize)
{
	return 0;
}

Gpu::ShaderLoader * Api::CreateGpuShaderLoader(Files::Directory * directory, const wchar_t * path)
{
	return new GL::ShaderLoader(this, files, directory, path);
}

Gpu::Mesh * Api::CreateGpuMesh(unsigned numVertices, void * vertexData, VertexType type, bool dynamic)
{
	GL::Mesh * createdMesh = new GL::Mesh();

	glGenVertexArrays(1, &(createdMesh->vertexArrayObject));
	glBindVertexArray(createdMesh->vertexArrayObject);

	glGenBuffers(1, &(createdMesh->vertexBufferObject));
	glBindBuffer(GL_ARRAY_BUFFER, createdMesh->vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, VertApi::GetVertexSize(type) * numVertices, vertexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	// Vertex specification for "PosCol" 
	// TODO: Add vertex attrib pointers for more vertex types!
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0); 
	// This shouldn't work? GL_ELEMENT_ARRAY_BUFFER (index buffer) should not be allowed unless a VAO is bound

	//createdMesh->vertexSize = VertApi::GetVertexSize(type);
	createdMesh->numVertices = numVertices;
	createdMesh->vertexType = type;
	createdMesh->dynamic = dynamic;

	return createdMesh;
}

Gpu::Mesh * Api::CreateGpuMesh(unsigned numVertices, void * vertexData,
	unsigned numTriangles, unsigned * indexData, VertexType type, bool dynamic)
{
	GL::Mesh * createdMesh = static_cast<GL::Mesh*>(CreateGpuMesh(numVertices, vertexData, type, dynamic));

	glGenBuffers(1, &(createdMesh->indexBufferObject));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, createdMesh->indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numTriangles * 3 * sizeof(unsigned), indexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	createdMesh->numTriangles = numTriangles;

	return createdMesh;
}

Gpu::InstanceBuffer * Api::CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type)
{
	return 0;
}

void Api::UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, IVertexBuffer * buffer)
{

}

void Api::UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, unsigned numTriangles, unsigned * indexData)
{

}

void Api::UpdateInstanceBuffer(Gpu::InstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData)
{

}

Gpu::DrawSurface * Api::CreateDrawSurface(unsigned width, unsigned height, Gpu::DrawSurface::Format format)
{
	return 0;
}

Gpu::DrawSurface * Api::CreateScreenDrawSurface(float widthFactor, float heightFactor, Gpu::DrawSurface::Format format)
{
	return 0;
}

void Api::AddDeviceListener(Gpu::IDeviceListener * listener)
{

}

void Api::RemoveDeviceListener(Gpu::IDeviceListener * listener)
{

}

void Api::BeginTimestamp(const std::wstring name)
{

}

void Api::EndTimestamp(const std::wstring name)
{

}

Gpu::TimestampData Api::GetTimestampData(const std::wstring name)
{
	return Gpu::TimestampData();
}

float Api::MeasureGpuText(Gpu::Font * font, const wchar_t * text)
{
	return 0.0f;
}

void Api::SetClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void Api::OnScreenResize(unsigned width, unsigned height)
{
	backbufferWidth = width;
	backbufferHeight = height;
}

void Api::GetBackbufferSize(unsigned & width, unsigned & height)
{
	width = backbufferWidth;
	height = backbufferHeight;
}

void Api::SetMultisampling(unsigned multisampleLevel)
{

}

void Api::SetBlendMode(Gpu::BlendMode blendMode)
{

}

void Api::SetAnisotropy(unsigned anisotropy)
{

}

bool Api::isDeviceLost()
{
	return false;
}

} // namespace GL
} // namespace Ingenuity

#endif // USE_GL_GPUAPI

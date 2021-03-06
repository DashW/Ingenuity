#ifdef USE_GL_GPUAPI

#include "GLApi.h"
#include "GLShaders.h"

#include <Windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

#include <SOIL.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB    0x9126
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

namespace Ingenuity {
namespace GL {

struct VertexAttribDesc
{
	unsigned numFloats;
	VertexComponent::Type component;
	unsigned interval;
	unsigned offset;
};

VertexAttribDesc posDesc[2] = {
		{ 3, VertexComponent::Pos, 0, 0 },
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posColDesc[3] = {
		{ 3, VertexComponent::Pos, 6 * sizeof(float), 0 },
		{ 3, VertexComponent::Col, 6 * sizeof(float), 3 * sizeof(float) },
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posNorDesc[3] = {
		{ 3, VertexComponent::Pos, 6 * sizeof(float), 0 },
		{ 3, VertexComponent::Nor, 6 * sizeof(float), 3 * sizeof(float) },
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posTexDesc[3] = {
		{ 3, VertexComponent::Pos, 5 * sizeof(float), 0 },
		{ 2, VertexComponent::Tex, 5 * sizeof(float), 3 * sizeof(float) },
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posNorTexDesc[4] = {
		{ 3, VertexComponent::Pos, 8 * sizeof(float), 0 },
		{ 3, VertexComponent::Nor, 8 * sizeof(float), 3 * sizeof(float) },
		{ 2, VertexComponent::Tex, 8 * sizeof(float), 6 * sizeof(float) },
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posNorTanTexDesc[5] = {
		{ 3, VertexComponent::Pos, 11 * sizeof(float), 0 },
		{ 3, VertexComponent::Nor, 11 * sizeof(float), 3 * sizeof(float) },
		{ 3, VertexComponent::Tan, 11 * sizeof(float), 6 * sizeof(float) },
		{ 2, VertexComponent::Tex, 11 * sizeof(float), 9 * sizeof(float) },
		{ 0, VertexComponent::Nil, 0, 0 }
};

VertexAttribDesc * vertexDescs[VertexType_Count] = {
	posDesc,
	posColDesc,
	posNorDesc,
	posTexDesc,
	posNorTexDesc,
	posNorTanTexDesc
};

VertexAttribDesc noneInstDesc[1] = {
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posInstDesc[2] = {
		{ 3, VertexComponent::InstPos, 0, 0 },
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posColInstDesc[3] = {
		{ 3, VertexComponent::InstPos, 7 * sizeof(float), 0 },
		{ 4, VertexComponent::InstCol, 7 * sizeof(float), 3 * sizeof(float) },
		{ 0, VertexComponent::Nil, 0, 0 }
};
VertexAttribDesc posScaInstDesc[3] = {
		{ 3, VertexComponent::InstPos, 6 * sizeof(float), 0 },
		{ 3, VertexComponent::InstSca, 6 * sizeof(float), 3 * sizeof(float) },
		{ 0, VertexComponent::Nil, 0, 0 }
};

VertexAttribDesc * instanceDescs[InstanceType_Count] = {
	0,
	posInstDesc,
	posColInstDesc,
	posScaInstDesc,
};

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &vertexArrayId);
	glDeleteBuffers(1, &vertexBufferId);
	if(indexBufferId)
	{
		glDeleteBuffers(1, &indexBufferId);
	}
}

InstanceBuffer::~InstanceBuffer()
{
	glDeleteBuffers(1, &instanceBufferId);
}

Texture::~Texture()
{
	glDeleteTextures(1, &textureId);
}

CubeMap::~CubeMap()
{
	glDeleteTextures(1, &cubeMapId);
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

	if(wglewIsSupported("WGL_ARB_create_context") == 1) 
	{
		glContext = wglCreateContextAttribsARB(deviceContext, 0, attributes);
		wglMakeCurrent(0, 0);
		wglDeleteContext(tempContext);
		wglMakeCurrent(deviceContext, glContext);

		const char * currentVersion = (const char*) glGetString(GL_VERSION);
		const char * currentExtensions = (const char*) glGetString(GL_EXTENSIONS);
		OutputDebugStringA(currentVersion);
		OutputDebugStringA("\n");
		OutputDebugStringA(currentExtensions);
		OutputDebugStringA("\n");
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

	bool wireframe = drawEverythingWireframe || model->wireframe;
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
	unsigned instanceBufferId = 0;
	if(instances)
	{
		instanceType = instances->GetType();
		instanceBufferId = static_cast<GL::InstanceBuffer*>(instances)->instanceBufferId;
	}

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

	glBindVertexArray(glMesh->vertexArrayId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh->indexBufferId);

	// Since the VertexArray contains bindings for both the vertex and index buffers,
	// it should be recreated for the mesh if the instance buffer changes...

	if(glMesh->lastInstanceBufferId != instanceBufferId)
	{
		for(unsigned i = 0; i < VertexComponent::Count; ++i)
		{
			glDisableVertexAttribArray(i);
		}

		glBindBuffer(GL_ARRAY_BUFFER, glMesh->vertexBufferId);
		VertexAttribDesc * desc = vertexDescs[glMesh->vertexType];
		for(unsigned i = 0; desc[i].component != VertexComponent::Nil; ++i)
		{
			glVertexAttribPointer(desc[i].component, desc[i].numFloats, GL_FLOAT, GL_FALSE, desc[i].interval, (char*)desc[i].offset);
			glVertexAttribDivisor(desc[i].component, 0);
			glEnableVertexAttribArray(desc[i].component);
		}

		if(instances)
		{
			GL::InstanceBuffer * glInstances = static_cast<GL::InstanceBuffer*>(instances);
			glBindBuffer(GL_ARRAY_BUFFER, glInstances->instanceBufferId);
			VertexAttribDesc * desc = instanceDescs[glInstances->type];
			for(unsigned i = 0; desc[i].component != VertexComponent::Nil; ++i)
			{
				glVertexAttribPointer(desc[i].component, desc[i].numFloats, GL_FLOAT, GL_FALSE, desc[i].interval, (char*)desc[i].offset);
				glVertexAttribDivisor(desc[i].component, 1);
				glEnableVertexAttribArray(desc[i].component);
			}
		}

		glMesh->lastInstanceBufferId = instanceBufferId;
	}

	//if(dx11surface) dx11surface->Begin();
	if(glMesh->IsIndexed())
	{
		if(instances)
		{
			glDrawElementsInstanced(GL_TRIANGLES, glMesh->numTriangles * 3, GL_UNSIGNED_INT, 0, instances->GetLength());
		}
		else
		{
			glDrawElements(GL_TRIANGLES, glMesh->numTriangles * 3, GL_UNSIGNED_INT, 0);
		}
	}
	else
	{
		if(instances)
		{
			glDrawArraysInstanced(GL_TRIANGLES, 0, glMesh->numVertices, instances->GetLength());
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, glMesh->numVertices);
		}
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
	GL::Texture * texture = new GL::Texture();

	texture->textureId = SOIL_load_OGL_texture_from_memory((const unsigned char*) data, dataSize, 4, 0, SOIL_FLAG_MIPMAPS);

	glBindTexture(GL_TEXTURE_2D, texture->textureId);

	int width, height;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	glBindTexture(GL_TEXTURE_2D, 0);

	texture->width = (unsigned)width;
	texture->height = (unsigned)height;
	
	return texture;
}

Gpu::CubeMap * Api::CreateGpuCubeMap(char * data, unsigned dataSize)
{
	GL::CubeMap * cubeMap = new GL::CubeMap();

	cubeMap->cubeMapId = SOIL_load_OGL_single_cubemap_from_memory((const unsigned char*) data, dataSize, "EWUDNS", 4, 0, 0);

	return cubeMap;
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

	glGenVertexArrays(1, &(createdMesh->vertexArrayId));
	glBindVertexArray(createdMesh->vertexArrayId);

	glGenBuffers(1, &(createdMesh->vertexBufferId));
	glBindBuffer(GL_ARRAY_BUFFER, createdMesh->vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, VertApi::GetVertexSize(type) * numVertices, vertexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	VertexAttribDesc * desc = vertexDescs[type];
	for(unsigned i = 0; desc[i].component != VertexComponent::Nil; ++i)
	{
		glVertexAttribPointer(desc[i].component, desc[i].numFloats, GL_FLOAT, GL_FALSE, desc[i].interval, (char*)desc[i].offset);
		glVertexAttribDivisor(desc[i].component, 0);
		glEnableVertexAttribArray(desc[i].component);
	}

	glBindVertexArray(0);

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

	glGenBuffers(1, &(createdMesh->indexBufferId));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, createdMesh->indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numTriangles * 3 * sizeof(unsigned), indexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	createdMesh->numTriangles = numTriangles;

	return createdMesh;
}

Gpu::InstanceBuffer * Api::CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type)
{
	GL::InstanceBuffer * instanceBuffer = new GL::InstanceBuffer(type);

	glGenBuffers(1, &(instanceBuffer->instanceBufferId));
	glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer->instanceBufferId);
	glBufferData(GL_ARRAY_BUFFER, VertApi::GetInstanceSize(type) * numInstances, instanceData, GL_DYNAMIC_DRAW);

	//instanceBuffer->numVertices = numVertices;
	instanceBuffer->length = numInstances;
	instanceBuffer->capacity = numInstances;

	return instanceBuffer;
}

void Api::UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, IVertexBuffer * buffer)
{
	GL::Mesh * glMesh = static_cast<GL::Mesh*>(dynamicMesh);

	// Need to fix this so that the mesh has vertex/index capacities?
	if(glMesh->dynamic &&
		buffer->GetVertexType() == glMesh->vertexType &&
		buffer->GetLength() <= glMesh->numVertices)
	{
		glBindBuffer(GL_ARRAY_BUFFER, glMesh->vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, buffer->GetElementSize() * buffer->GetLength(), buffer->GetData(), GL_DYNAMIC_DRAW);
	}
}

void Api::UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, unsigned numTriangles, unsigned * indexData)
{
	GL::Mesh * glMesh = static_cast<GL::Mesh*>(dynamicMesh);

	if(glMesh->dynamic &&
		numTriangles <= glMesh->numTriangles)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh->indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * numTriangles * 3, indexData, GL_DYNAMIC_DRAW);
	}
}

void Api::UpdateInstanceBuffer(Gpu::InstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData)
{
	GL::InstanceBuffer * glInstances = static_cast<GL::InstanceBuffer*>(instanceBuffer);

	if(glInstances->capacity < numInstances) return;

	glBindBuffer(GL_ARRAY_BUFFER, glInstances->instanceBufferId);
	glBufferData(GL_ARRAY_BUFFER, VertApi::GetInstanceSize(glInstances->GetType()) * numInstances, instanceData, GL_DYNAMIC_DRAW);
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
	//int attributes[] = {
	//	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	//	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	//	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	//	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	//	WGL_COLOR_BITS_ARB, 32,
	//	WGL_DEPTH_BITS_ARB, 32,
	//	WGL_STENCIL_BITS_ARB, 0,
	//	WGL_SAMPLE_BUFFERS_ARB, 1, //Number of buffers (must be 1 at time of writing)
	//	WGL_SAMPLES_ARB, 4,        //Number of samples
	//	0
	//};
	//float fAttributes[] = { 0, 0 };

	//int pixelFormat;
	//unsigned numFormats;

	//wglChoosePixelFormatARB(deviceContext, attributes, fAttributes, 1, &pixelFormat, &numFormats);

	////apparently we need to recreate the window at this point, but this seems like overkill??
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

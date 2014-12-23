#pragma once

#include "FilesApi.h"
#include "AssetMgr.h"
#include "GpuShaders.h"
#include "GpuVertices.h"
#include "StepMgr.h"

namespace Ingenuity {

class PlatformWindow;

namespace Gpu {

class Api;

struct ShaderLoader;
struct MeshCopy;
struct LocalMesh;

struct Drawable
{
	virtual void BeDrawn(Api * gpu, DrawSurface * surface = 0) = 0;
};

enum BlendMode
{
	BlendMode_None,
	BlendMode_Alpha,
	BlendMode_Additive
};

/* Abstract class to interact with a graphics API like DirectX or OpenGL */
class Api
{
protected:
	bool drawEverythingWireframe;

	DrawSurface * stencilSurface;
	DrawSurface * stencilClipSurface;

public:
	const unsigned standardScreenHeight;

	Api() :
		drawEverythingWireframe(false),
		standardScreenHeight(768),
		stencilSurface(0),
		stencilClipSurface(0) {}
	virtual ~Api() {};

	virtual void Initialize(AssetMgr * assets) = 0;
	virtual void OnCriticalLoad(AssetMgr * assets) = 0;
	virtual void Clear() = 0;
	virtual void BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void Present() = 0;

	inline void Draw(Drawable * drawable, DrawSurface * surface = 0) { drawable->BeDrawn(this, surface); }

	virtual void DrawGpuSprite(Sprite * sprite, DrawSurface * buffer = 0) = 0;
	virtual void DrawGpuText(Font * font, const wchar_t* text, float x, float y,
		bool center, DrawSurface * surface = 0) = 0;
	virtual void DrawGpuModel(Model * model, Camera * camera, Light ** lights,
		unsigned numLights, DrawSurface * surface = 0, InstanceBuffer * instances = 0, Effect * overrideEffect = 0) = 0;
	virtual void DrawGpuSurface(DrawSurface * source, Effect * effect, DrawSurface * dest) = 0;

	virtual Font * CreateGpuFont(int height, const wchar_t * facename, FontStyle style = FontStyle_Regular) = 0;
	virtual Texture * CreateGpuTexture(char * data, unsigned dataSize, bool isDDS = false) = 0;
	virtual CubeMap * CreateGpuCubeMap(char * data, unsigned dataSize) = 0;
	virtual VolumeTexture * CreateGpuVolumeTexture(char * data, unsigned dataSize) = 0;
	virtual ShaderLoader * CreateGpuShaderLoader(Files::Directory * directory, const wchar_t * path) = 0;

	virtual Mesh * CreateGpuMesh(unsigned numVertices, void * vertexData, VertexType type, bool dynamic = false) = 0;
	virtual Mesh * CreateGpuMesh(unsigned numVertices, void * vertexData,
		unsigned numTriangles, unsigned * indexData, VertexType type, bool dynamic = false) = 0;

	Mesh * CreateGpuMesh(IVertexBuffer * buffer, bool dynamic = false)
	{
		return CreateGpuMesh(
			buffer->GetLength(),
			buffer->GetData(),
			buffer->GetVertexType(),
			dynamic);
	}
	Mesh * CreateGpuMesh(IVertexBuffer * buffer, unsigned numTriangles, unsigned * indexData, bool dynamic = false)
	{
		return CreateGpuMesh(
			buffer->GetLength(),
			buffer->GetData(),
			numTriangles,
			indexData,
			buffer->GetVertexType(),
			dynamic);
	}

	virtual InstanceBuffer * CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type) = 0;

	virtual void UpdateDynamicMesh(Mesh * dynamicMesh, IVertexBuffer * buffer) = 0;
	virtual void UpdateDynamicMesh(Mesh * dynamicMesh, unsigned numTriangles, unsigned * indexData) = 0;
	virtual void UpdateInstanceBuffer(InstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData) = 0;

	DrawSurface * GetStencilSurface() { return stencilSurface; }
	DrawSurface * GetStencilClipSurface() { return stencilClipSurface; }
	virtual DrawSurface * CreateDrawSurface(unsigned width, unsigned height, DrawSurface::Format format = DrawSurface::Format_4x8int) = 0;
	virtual DrawSurface * CreateRelativeDrawSurface(float widthFactor = 1.0f, float heightFactor = 1.0f, DrawSurface::Format format = DrawSurface::Format_4x8int, PlatformWindow * window = 0) = 0;
	virtual DrawSurface * GetWindowDrawSurface(PlatformWindow * window) = 0;

	virtual void AddDeviceListener(IDeviceListener * listener) = 0;
	virtual void RemoveDeviceListener(IDeviceListener * listener) = 0;

	virtual void BeginTimestamp(const std::wstring name) = 0;
	virtual void EndTimestamp(const std::wstring name) = 0;
	virtual TimestampData GetTimestampData(const std::wstring name) = 0;

	virtual float MeasureGpuText(Font * font, const wchar_t * text) = 0; // DEPRECATE when removing Font API!
	virtual void SetClearColor(float r, float g, float b, float a) = 0;

	virtual void OnWindowCreated(PlatformWindow * window) = 0;
	virtual void OnWindowResized(PlatformWindow * window, unsigned width, unsigned height) = 0;
	virtual void OnWindowDestroyed(PlatformWindow * window) = 0;
	virtual void GetBackbufferSize(unsigned & width, unsigned & height, PlatformWindow * window = 0) = 0;
	virtual void SetMultisampling(unsigned multisampleLevel) = 0;
	virtual void SetBlendMode(BlendMode blendMode) = 0;

	unsigned GetStandardScreenHeight() { return standardScreenHeight; } // DEPRECATE when removing Sprite API!
	virtual void SetDrawWireframe(bool wireframe) { drawEverythingWireframe = wireframe; }

	virtual void SetAnisotropy(unsigned anisotropy) = 0;

	virtual bool isDeviceLost() = 0;
};

struct Sprite : public Drawable
{
	Texture * texture;
	 
	glm::vec2 center;
	glm::vec3 position;
	glm::vec2 scale;
	glm::vec4 color;

	float rotation;
	bool pixelSpace;
	bool brightAsAlpha;
	Rect clipRect;

	Sprite(
		Texture * texture = 0,
		float transformCenterX = 0.0f,
		float transformCenterY = 0.0f)
		: texture(texture), center(transformCenterX, transformCenterY),
		rotation(0.0f), scale(1.0f, 1.0f), color(1.0f, 1.0f, 1.0f, 1.0f),
		pixelSpace(false), brightAsAlpha(false),
		clipRect(0.0f, 0.0f, 1.0f, 1.0f) {}

	virtual void BeDrawn(Api* gpu, DrawSurface * surface = 0) override
	{
		gpu->DrawGpuSprite(this, surface);
	}
};

struct ComplexModel : public IAsset
{
	Model * models;
	unsigned numModels;

	glm::vec4 position;
	glm::vec4 rotation;
	glm::vec4 scale; 
	glm::vec4 matrixPadding;

	bool useMatrix;
	bool wireframe;

	inline void SetMatrix(glm::mat4& matrix)
	{
		position = matrix[0];
		rotation = matrix[1];
		scale = matrix[2];
		matrixPadding = matrix[3];
		useMatrix = true;
	}
	inline glm::mat4 GetMatrix()
	{
		if(useMatrix)
		{
			return glm::mat4(position, rotation, scale, matrixPadding);
		}
		else
		{
			return glm::translate(glm::vec3(position))
				* glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z)
				* glm::scale(glm::vec3(scale.x));
		}
	}

	InstanceBuffer * instances;

	ComplexModel(int numModels) :
		models(0),
		numModels(numModels),
		scale(1.0f),
		useMatrix(false),
		wireframe(false),
		instances(0)
	{
		models = new Model[numModels];
	}

	virtual ~ComplexModel()
	{
		delete[] models;
	}

	virtual AssetType GetType() override { return ModelAsset; }
	virtual IAsset * GetAsset() override
	{
		ComplexModel * dst = new ComplexModel(numModels);
		for(unsigned i = 0; i < numModels; ++i)
		{
			dst->models[i] = models[i];
			dst->models[i].destructMesh = false;
		}
		return dst;
	}

	virtual void BeDrawn(Api * gpu, Camera * camera, Light ** lights,
		unsigned numLights, DrawSurface * buffer = 0, Gpu::Effect * overrideEffect = 0)
	{
		Model tempModel;

		for(unsigned i = 0; i < numModels; ++i)
		{
			tempModel = models[i];
			if(useMatrix || tempModel.useMatrix)
			{
				tempModel.SetMatrix(tempModel.GetMatrix() * GetMatrix());
			}
			else
			{
				tempModel.position += position; // with respect to rotation about the parent model's axis??
				tempModel.rotation += rotation;
				tempModel.scale *= scale;
			}
			tempModel.wireframe = wireframe ? true : tempModel.wireframe;
			tempModel.destructMesh = false;

			gpu->DrawGpuModel(&tempModel, camera, lights, numLights, buffer, instances, overrideEffect);
		}
	}
};

struct TextureLoader : public SimpleLoader
{
	Api * gpu;

	Texture * texture;
	CubeMap * cubeMap;
	VolumeTexture * volumeTex;

	TextureLoader(Api * gpu, Files::Api * files, Files::Directory * directory,
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

	Texture * GetTexture() { return texture; }
	CubeMap * GetCubeTex() { return cubeMap; }
	VolumeTexture * GetVolumeTex() { return volumeTex; }

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

struct MeshCopy : public Steppable
{
	LocalMesh * localMesh;

	MeshCopy(LocalMesh * localMesh) : localMesh(localMesh) {}

	virtual Mesh * GetMesh() = 0;
};

} // namespace Gpu
} // namespace Ingenuity

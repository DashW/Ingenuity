#pragma once

#define GLM_FORCE_RADIANS
#include "../Third Party/glm-0.9.5.4/glm/glm.hpp"
#include "../Third Party/glm-0.9.5.4/glm/gtx/euler_angles.hpp"
#include "../Third Party/glm-0.9.5.4/glm/gtx/transform.hpp"
#include "AssetMgr.h"
#include "GpuShaders.h"
#include <string>

namespace Ingenuity {

class IVertexBuffer;
enum VertexType;
enum InstanceType;

namespace Gpu {

class Api;
struct Drawable;
struct Sprite;

struct Rect
{
	float left;
	float top;
	float right;
	float bottom;

	Rect(float l, float t, float r, float b)
		: left(l), top(t), right(r), bottom(b) {}
	Rect()
		: left(0.0f), top(0.0f), right(0.0f), bottom(0.0f) {}
};

// Override this to respond to Lost or Reset Device events
struct IDeviceListener {
	virtual void OnLostDevice(Api * gpu) = 0;
	virtual void OnResetDevice(Api * gpu) = 0;
};

struct Font
{
	glm::vec4 color;
	bool pixelSpace;

	virtual ~Font() {};
protected:
	Font() : color(0.0f, 0.0f, 0.0f, 1.0f), pixelSpace(true) {}
};

enum FontStyle
{
	FontStyle_Regular,
	FontStyle_Bold,
	FontStyle_Italic
};

struct Texture : public IAsset {
	virtual ~Texture() {}
	std::wstring texPath;

	virtual unsigned GetWidth() = 0;
	virtual unsigned GetHeight() = 0;
	virtual AssetType GetType() override { return TextureAsset; }
	virtual IAsset * GetAsset() override { return this; }

protected:
	Texture() {}
};

struct CubeMap : public IAsset {
	virtual ~CubeMap() {}
	virtual AssetType GetType() override { return CubeMapAsset; }
	virtual IAsset * GetAsset() override { return this; }
protected:
	CubeMap(){}
};

struct VolumeTexture : public IAsset {
	virtual ~VolumeTexture() {}
	virtual AssetType GetType() override { return VolumeTexAsset; }
	virtual IAsset * GetAsset() override { return this; }
protected:
	VolumeTexture(){}
};

struct Mesh {
	virtual ~Mesh() {}
protected:
	Mesh(){}
};

struct Effect;

// HACK
struct BoundingBox
{
	glm::vec3 origin;
	glm::vec3 dimensions;
};
struct BoundingSphere
{
	glm::vec3 origin;
	float radius;

	BoundingSphere() : radius(0) {}
};

struct Model
{
	Mesh * mesh;
	Texture * texture;
	Texture * normalMap; // LIGHTING
	CubeMap * cubeMap;   // LIGHTING
	Effect * effect;

	glm::vec4 position;
	glm::vec4 rotation;
	glm::vec4 scale;
	glm::vec4 matrixPadding;
	glm::vec4 color;

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
				* glm::scale(glm::vec3(scale));
		}
	}

	float diffuseFactor; // LIGHTING
	float specPower;     // LIGHTING
	float specFactor;    // LIGHTING

	bool useMatrix;
	bool wireframe;
	bool wrapTexture;
	bool backFaceCull;
	bool frontFaceCull;
	bool destructMesh;
	bool destructEffect;

	// HACK
	BoundingSphere boundingSphere;

	Model() :
		mesh(0), texture(0), normalMap(0), cubeMap(0), effect(0),
		position(0.0f,0.0f,0.0f,1.0f), rotation(0.0f), scale(1.0f), matrixPadding(0.0f), color(1.0f), 
		diffuseFactor(1.0f), specPower(16.0f), specFactor(1.0f), useMatrix(false),
		wrapTexture(false), wireframe(false), backFaceCull(true), frontFaceCull(false), 
		destructMesh(false), destructEffect(false) {}
	virtual ~Model()
	{
		if(destructMesh) delete mesh;
		if(effect && destructEffect) delete effect;
	}
};

enum LightType
{
	LightType_Directional = 0,
	LightType_Point,
	LightType_Spot
};

struct Light
{
	glm::vec3 color;

	Light() : color(1.0f) {}
	virtual ~Light() {}
	virtual LightType GetType() = 0;

	static float DefaultAtten() { return 0.0f; }
};

struct DirectionalLight : public Light
{
	glm::vec3 direction;

	virtual LightType GetType() override
	{
		return LightType_Directional;
	}
};

struct PointLight : public Light
{
	glm::vec3 position;
	float atten;

	PointLight() : atten(Light::DefaultAtten()) {}
	virtual ~PointLight() {}

	virtual LightType GetType() override
	{
		return LightType_Point;
	}
};

struct SpotLight : public Light
{
	glm::vec3 position;
	glm::vec3 direction;

	float atten;
	float power;

	glm::mat4 GetMatrix(float fov, glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f))
	{
		return glm::perspective(fov, 1.0f, 1.0f, 200.0f)
			* (glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) 
			*  glm::lookAt(position, position + direction, up));
	}

	SpotLight() : atten(Light::DefaultAtten()), power(12.0f) {}

	virtual LightType GetType() override
	{
		return LightType_Spot;
	}
};

struct Camera
{
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	float fovOrHeight, nearClip, farClip;
	bool isOrthoCamera;

	inline glm::mat4 GetViewMatrix()
	{
		return glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) * glm::lookAt(position, target, up);
	}

	inline glm::mat4 GetProjMatrix(float aspect)
	{
		const float halfY = fovOrHeight * 0.5f;
		const float halfX = fabs(halfY) * aspect;

		// glm::ortho returns Z coordinates in the range -1 to 1.
		// This matrix will transform them to the correct range 0 to 1.
		static const glm::mat4 orthoFixMatrix = glm::translate(glm::vec3(0.0f, 0.0f, 0.5f)) * glm::scale(glm::vec3(1.0f, 1.0f, 0.5f));

		return isOrthoCamera ?
			orthoFixMatrix * glm::ortho(-halfX, halfX, -halfY, halfY, nearClip, farClip) :
			glm::perspective(fovOrHeight, aspect, nearClip, farClip);
	}

	inline glm::vec3 GetUnprojectedRay(float x, float y, float aspect)
	{
		glm::mat4 viewMatrix = GetViewMatrix();
		viewMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// How did I figure out to use the inverse-transpose view matrix here?

		return glm::normalize(glm::unProject(
			glm::vec3(x, 1.0f-y, 0.0f), 
			glm::inverse(glm::transpose(viewMatrix)), 
			GetProjMatrix(aspect), 
			glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)));
	}

	Camera() :
		position(0.0f, 0.0f, -1.0f),
		target(0.0f, 0.0f, 0.0f),
		up(0.0f, 1.0f, 0.0f),
		fovOrHeight(0.78539f),
		nearClip(1.0f), farClip(5000.0f),
		isOrthoCamera(false) {}
};

struct DrawSurface
{
	enum Type
	{
		TypeBackbuffer,
		TypeTexture,
		TypeRelativeTexture,
		TypeStencil,
		TypeStencilClip
	};

	enum Format
	{
		Format_4x8int,
		Format_4x16float,
		Format_3x10float,
		Format_1x16float,
		Format_Typeless,

		Format_Total
	};

	virtual ~DrawSurface() {}

	virtual Type GetSurfaceType() = 0;
	virtual Texture * GetTexture() = 0;
	virtual void Clear(glm::vec4 & color = glm::vec4(0.0f,0.0f,0.0f,1.0f)) = 0;
	virtual unsigned GetWidth() = 0;
	virtual unsigned GetHeight() = 0;
};

struct InstanceBuffer
{
	InstanceType type;

	virtual unsigned GetLength() = 0;
	virtual unsigned GetCapacity() = 0;

	InstanceType GetType() { return type; }

	virtual ~InstanceBuffer() {}
protected:
	InstanceBuffer(InstanceType type) : type(type) {}
};

struct TimestampData
{
	enum Type
	{
		Time,
		DrawCalls,
		Polys,
		StateChanges,
		Overhead,
		Other
	};

	std::map<Type, float> data;
};

//struct Blob
//{
//	void * data;
//	unsigned dataSize;
//
//	Blob(void * data, unsigned dataSize) :
//		data(data), dataSize(dataSize) {}
//};

} // namespace Gpu
} // namespace Ingenuity

#pragma once

#include "../Third Party/glm-0.9.4.1/glm/glm.hpp"
#include "AssetMgr.h"
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
	Texture * normalMap;
	CubeMap * cubeMap;
	Effect * effect;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec4 color;

	float diffuseFactor;
	float specPower;
	float specFactor;

	bool wireframe;
	bool wrapTexture;
	bool backFaceCull;
	bool frontFaceCull;
	bool destructMesh;

	// HACK
	BoundingSphere boundingSphere;

	Model() :
		mesh(0), texture(0), normalMap(0), cubeMap(0), effect(0), color(1.0f, 1.0f, 1.0f, 1.0f),
		scale(1.0f, 1.0f, 1.0f), diffuseFactor(1.0f), specPower(16.0f), specFactor(1.0f),
		wrapTexture(false), wireframe(false), backFaceCull(true), frontFaceCull(false), destructMesh(false) {}
	virtual ~Model()
	{
		if(destructMesh) delete mesh;
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

	SpotLight() : atten(Light::DefaultAtten()), power(0) {}

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
		TypeTexture,
		TypeFullscreenTexture,
		TypeStencil,
		TypeStencilClip
	};

	enum Format
	{
		Format_4x8int,
		Format_4x16float,
		Format_1x16float,

		Format_Total
	};

	virtual ~DrawSurface() {}

	virtual Type GetSurfaceType() = 0;
	virtual Texture * GetTexture() = 0;
	virtual void Clear(glm::vec4 color = glm::vec4(0.0f,0.0f,0.0f,1.0f)) = 0;
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

struct Timestamp
{
	virtual ~Timestamp() {}
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

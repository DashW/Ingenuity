#pragma once

#include "GpuApi.h"

namespace Ingenuity {

// HOW ON EARTH AM I GOING TO GET ALL THIS BOUND TO THE SCRIPT ENVIRONMENT???

struct Sprite
{
	Gpu::Texture * texture;

	//glm::vec2 pivot;
	glm::vec3 position;
	glm::vec2 scale;
	glm::vec4 color;
	
	glm::vec2 uvOffset;
	glm::vec2 uvScale;

	float rotation;
	float uvRotation;

	bool brightAsAlpha;

	Sprite(Gpu::Texture * texture = 0/*, glm::vec2 pivot = glm::vec2(0.0f)*/)
		: texture(texture)
		//, pivot(pivot)
		, rotation(0.0f)
		, scale(1.0f, 1.0f)
		, color(1.0f, 1.0f, 1.0f, 1.0f)
		, brightAsAlpha(false)
	{}
};

struct SpriteCamera
{
	glm::vec2 position;
	float rotation;
	float scale;

	bool pixelSpace;
	bool centerOrigin;
	bool yUpwards;

	SpriteCamera()
		: rotation(0.0f)
		, scale(1.0f)
		, pixelSpace(true)
		, centerOrigin(false)
		, yUpwards(false)
	{}
};

class SpriteMgr
{
	Gpu::Api * gpu;
	Gpu::Model * quad;
	Gpu::InstanceBuffer * instanceBuffer;

public:
	SpriteMgr(Gpu::Api * gpu);
	~SpriteMgr();

	Gpu::Mesh * GetQuadMesh();
	Gpu::Camera SpriteToGpuCamera(SpriteCamera & spriteCam, Gpu::DrawSurface * surface);

	void DrawSprite(Sprite * sprite, SpriteCamera & spriteCam = SpriteCamera(), Gpu::DrawSurface * surface = 0);
	void DrawSprite(Sprite * sprite, Gpu::Camera * camera, Gpu::DrawSurface * surface = 0);
	//void DrawBatch(Sprite * sprite, unsigned numSprites, SpriteCamera & spriteCam = SpriteCamera(), Gpu::DrawSurface * surface = 0);
	//void DrawBatch(Sprite * sprite, unsigned numSprites, Gpu::Camera & camera, Gpu::DrawSurface * surface = 0);

	// Look at the DX11 SpriteBatch for ideas on how to do batching

};

} // namespace Ingenuity

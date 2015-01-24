#include "SpriteManager.h"
#include "GeoBuilder.h"

namespace Ingenuity {

SpriteMgr::SpriteMgr(Gpu::Api * gpu)
	: gpu(gpu)
	, quad(0)
	, instanceBuffer(0)
{
	LocalMesh * quadGeom = GeoBuilder().BuildRect(0.0f, 0.0f, 1.0f, 1.0f, true);
	quad = new Gpu::Model();
	quad->mesh = quadGeom->GpuOnly(gpu);
	quad->destructMesh = true;
}

SpriteMgr::~SpriteMgr()
{
	if(quad) delete quad;
	if(instanceBuffer) delete instanceBuffer;
}

Gpu::Mesh * SpriteMgr::GetQuadMesh()
{
	return quad->mesh;
}

Gpu::Camera SpriteMgr::SpriteToGpuCamera(SpriteCamera & spriteCam, Gpu::DrawSurface * surface)
{
	Gpu::Camera camera;
	camera.isOrthoCamera = true;

	glm::vec2 spaceSize;
	if(surface)
	{
		spaceSize = glm::vec2(surface->GetWidth(), surface->GetHeight());
	}
	else
	{
		unsigned width, height;
		gpu->GetBackbufferSize(width, height);
		spaceSize = glm::vec2(width, height);
	}

	if(!spriteCam.pixelSpace)
	{
		spaceSize.x = spaceSize.x / spaceSize.y;
		spaceSize.y = 1.0f;
	}

	if(spriteCam.yUpwards)
	{
		camera.fovOrHeight = spaceSize.y;
	}
	else
	{
		camera.fovOrHeight = -spaceSize.y;
	}

	camera.fovOrHeight /= spriteCam.scale;

	if(!spriteCam.centerOrigin)
	{
		camera.position.x = spaceSize.x / 2.0f;
		camera.position.y = spaceSize.y / 2.0f;
	}

	camera.position.x += spriteCam.position.x;
	camera.position.y += spriteCam.position.y;

	camera.target.x = camera.position.x;
	camera.target.y = camera.position.y;

	camera.up.x = sinf(spriteCam.rotation);
	camera.up.y = cosf(spriteCam.rotation);

	return camera;
}

void SpriteMgr::DrawSprite(Sprite * sprite, SpriteCamera & spriteCam, Gpu::DrawSurface * surface)
{
	DrawSprite(sprite, &SpriteToGpuCamera(spriteCam, surface), surface);
}

void SpriteMgr::DrawSprite(Sprite * sprite, Gpu::Camera * camera, Gpu::DrawSurface * surface)
{
	if(!sprite || !sprite->texture) return;

	// Alpha test and reference value.
	//direct3Ddevice->SetRenderState(D3DRS_ALPHAREF, 10);
	//direct3Ddevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	//if(sprite->brightAsAlpha)
	//{
	//	direct3Ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
	//	direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	//	direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	//}
	//else
	//{
	//	direct3Ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//	direct3Ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
	//	direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	//	direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//}

	//direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	//direct3Ddevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	glm::vec2 textureSize(sprite->texture->GetWidth(), sprite->texture->GetHeight());

	quad->texture = sprite->texture;
	quad->useMatrix = false;
	quad->position = glm::vec4(sprite->position, 1.0f);
	quad->rotation = glm::vec4(0.0f, 0.0f, sprite->rotation, 0.0f);
	quad->scale = glm::vec4(sprite->scale * textureSize, 1.0f, 1.0f);
	quad->color = sprite->color;

	if(camera->fovOrHeight < 0.0f)
	{
		quad->scale.y *= -1.0f;
	}

	//RECT sourceRect;
	//sourceRect.left = (LONG)(dx9tex->width  * sprite->clipRect.left);
	//sourceRect.top = (LONG)(dx9tex->height * sprite->clipRect.top);
	//sourceRect.right = (LONG)(dx9tex->width  * sprite->clipRect.right);
	//sourceRect.bottom = (LONG)(dx9tex->height * sprite->clipRect.bottom);

	gpu->DrawGpuModel(quad, camera, 0, 0, surface);
}


} // namespace Ingenuity

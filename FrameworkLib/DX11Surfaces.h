#pragma once

#include "DX11Api.h"

namespace Ingenuity {
namespace DX11 {

struct DrawSurface : public Gpu::DrawSurface
{
	ID3D11Device * device;
	ID3D11DeviceContext * context;

	DrawSurface(ID3D11Device * device, ID3D11DeviceContext * context) : device(device), context(context) {}
	virtual ~DrawSurface() {}

	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual Gpu::Texture * GetTexture() = 0;
};

struct TextureSurface : public DrawSurface, public Gpu::IDeviceListener
{
	// Assumes that the user will not want to have direct control of the viewport,
	// Should always match the surface resolution and camera clip panes

	DX11::Api * gpu;
	DX11::Texture * texture;
	ID3D11RenderTargetView * renderTargetView;
	ID3D11Texture2D * depthStencilTexture;
	ID3D11DepthStencilView * depthStencilView;
	D3D11_VIEWPORT viewport;
	Format format;
	float widthFactor;
	float heightFactor;
	bool screenRelative;
	bool generateMips;

	TextureSurface(DX11::Api * gpu, ID3D11Device * device, ID3D11DeviceContext * context,
		Format format, bool screenRelative, float width, float height, bool mips = false);
	virtual ~TextureSurface();

	virtual void Begin() override;
	virtual void End() override;
	virtual void OnLostDevice(Gpu::Api * gpu) override;
	virtual void OnResetDevice(Gpu::Api * gpu) override;
	virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) override;
	virtual Gpu::DrawSurface::Type GetSurfaceType() override;
	virtual Gpu::Texture * GetTexture() override { return texture; }
};

struct StencilSurface : public DX11::DrawSurface
{
	StencilSurface(ID3D11Device * device, ID3D11DeviceContext * context) : DX11::DrawSurface(device, context), stencilState(0) {}
	virtual ~StencilSurface() { if(stencilState) stencilState->Release(); }
	virtual void Begin() override;
	virtual void End() override;
	virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) override;
	virtual Gpu::DrawSurface::Type GetSurfaceType() override { return Gpu::DrawSurface::TypeStencil; }
	virtual Gpu::Texture * GetTexture() override { return 0; }

	ID3D11DepthStencilState * stencilState;
};

struct StencilClipSurface : public DX11::DrawSurface
{
	StencilClipSurface(ID3D11Device * device, ID3D11DeviceContext * context) : DX11::DrawSurface(device, context), stencilClipState(0) {}
	virtual ~StencilClipSurface() { if(stencilClipState) stencilClipState->Release(); }
	virtual void Begin() override;
	virtual void End() override;
	virtual void Clear(glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) override;
	virtual Gpu::DrawSurface::Type GetSurfaceType() override { return Gpu::DrawSurface::TypeStencilClip; }
	virtual Gpu::Texture * GetTexture() override { return 0; }

	ID3D11DepthStencilState * stencilClipState;
};

} // namespace DX11
} // namespace Ingenuity

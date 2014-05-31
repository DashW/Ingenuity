#pragma once

#include "DX9Api.h"

struct DX9_GpuDrawSurface : public GpuDrawSurface
{
	IDirect3DDevice9 * device;

	DX9_GpuDrawSurface(IDirect3DDevice9 * device) : device(device) {}
	virtual ~DX9_GpuDrawSurface() {}

	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual GpuTexture * GetTexture() = 0;
};

struct DX9_GpuTextureSurface : public DX9_GpuDrawSurface, IGpuDeviceListener
{
	// Assumes that the user will not want to have direct control of the viewport

	ID3DXRenderToSurface * surfaceObject;
	IDirect3DSurface9 * surfaceLevel;
	D3DVIEWPORT9 viewport;
	GpuApi * gpu;
	IDirect3DTexture9 * texObject;
	DX9_GpuTexture * texture;
	bool fullscreen;

	DX9_GpuTextureSurface(GpuApi * gpu, IDirect3DDevice9 * device, 
		bool fullscreen, unsigned width, unsigned height);
	virtual ~DX9_GpuTextureSurface();

	virtual void Begin() override;
	virtual void End() override;
	virtual void OnResetDevice(GpuApi * gpu) override;
	virtual void OnLostDevice(GpuApi * gpu) override;
	virtual void Clear() override;
	virtual GpuDrawSurface::Type GetSurfaceType() override;
	virtual GpuTexture * GetTexture() override { return texture; }
};

struct DX9_GpuStencilSurface : public DX9_GpuDrawSurface
{
	DX9_GpuStencilSurface(IDirect3DDevice9 * device) : DX9_GpuDrawSurface(device) {}
	virtual ~DX9_GpuStencilSurface() {}
	virtual void Begin() override;
	virtual void End() override;
	virtual void Clear() override;
	virtual GpuDrawSurface::Type GetSurfaceType() override { return GpuDrawSurface::Stencil; }
	virtual GpuTexture * GetTexture() override { return 0; }
};

struct DX9_GpuStencilClipSurface : public DX9_GpuDrawSurface
{
	DX9_GpuStencilClipSurface(IDirect3DDevice9 * device) : DX9_GpuDrawSurface(device) {}
	virtual ~DX9_GpuStencilClipSurface() {}
	virtual void Begin() override;
	virtual void End() override;
	virtual void Clear() override;
	virtual GpuDrawSurface::Type GetSurfaceType() override { return GpuDrawSurface::StencilClip; }
	virtual GpuTexture * GetTexture() override { return 0; }
};

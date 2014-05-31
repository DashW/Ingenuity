#ifdef USE_DX9_GPUAPI

#include "DX9Surfaces.h"
#include "DX9Api.h"

DX9_GpuTextureSurface::DX9_GpuTextureSurface(GpuApi * gpu, IDirect3DDevice9 * device, 
											 bool fullscreen, unsigned width, unsigned height) : 
	DX9_GpuDrawSurface(device), gpu(gpu), surfaceObject(0), surfaceLevel(0), texture(0), fullscreen(fullscreen)
{
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinZ = FLT_MAX;
	viewport.MaxZ = 0.0f;

	OnResetDevice(gpu);
	gpu->AddDeviceListener(this);
}
DX9_GpuTextureSurface::~DX9_GpuTextureSurface()
{
	OnLostDevice(gpu);
	gpu->RemoveDeviceListener(this);
}
void DX9_GpuTextureSurface::Begin()
{
	device->EndScene();
	HRESULT hr = surfaceObject->BeginScene(surfaceLevel, &viewport);
	if(!SUCCEEDED(hr))
	{
		OutputDebugString(L"Failed to begin scene\n");
	}
}
void DX9_GpuTextureSurface::End()
{
	surfaceObject->EndScene(D3DX_FILTER_NONE);
	device->BeginScene();
}
void DX9_GpuTextureSurface::OnResetDevice(GpuApi * gpu)
{
	OnLostDevice(gpu);

	IDirect3DTexture9 * texObject;

	UINT usage = D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP;
	if(fullscreen) 
	{
		unsigned screenWidth = 0, screenHeight = 0;
		gpu->GetBackbufferSize(screenWidth, screenHeight);
		viewport.Width = screenWidth;
		viewport.Height = screenHeight;
	}

	D3DXCreateTexture(device, viewport.Width, viewport.Height, 0, usage, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texObject);
	D3DXCreateRenderToSurface(device, viewport.Width, viewport.Height, D3DFMT_A8R8G8B8, true, D3DFMT_D24S8, &surfaceObject);
	texObject->GetSurfaceLevel(0, &surfaceLevel);

	texture = new DX9_GpuTexture(texObject,viewport.Width,viewport.Height);

	Clear();
}
void DX9_GpuTextureSurface::OnLostDevice(GpuApi * gpu)
{
	if(texture)
	{
		delete texture; texture = 0;
		surfaceObject->Release(); surfaceObject = 0;
		surfaceLevel->Release(); surfaceLevel = 0;
	}
}
void DX9_GpuTextureSurface::Clear()
{
	Begin();
	HRESULT hr = device->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
	if(FAILED(hr))
	{
		DebugBreak();
	}
	End();
}
GpuDrawSurface::Type DX9_GpuTextureSurface::GetSurfaceType()
{
	return fullscreen ? GpuDrawSurface::FullscreenTexture : GpuDrawSurface::Texture; 
}

void DX9_GpuStencilSurface::Begin()
{
	device->SetRenderState( D3DRS_STENCILENABLE,	true );
	device->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_ALWAYS );
	device->SetRenderState( D3DRS_STENCILREF,		0x1 );
	device->SetRenderState( D3DRS_STENCILMASK,		0xffffffff );
	device->SetRenderState( D3DRS_STENCILWRITEMASK,	0xffffffff );
	device->SetRenderState( D3DRS_STENCILZFAIL,		D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_REPLACE );
	device->SetRenderState( D3DRS_ZWRITEENABLE,		false );
}
void DX9_GpuStencilSurface::End()
{
	device->SetRenderState( D3DRS_ZWRITEENABLE, true );
	device->SetRenderState( D3DRS_STENCILENABLE, false );
}
void DX9_GpuStencilSurface::Clear()
{
	device->Clear(0, 0, D3DCLEAR_STENCIL, 0, 1.0f, 0);
}

void DX9_GpuStencilClipSurface::Begin()
{
	device->SetRenderState( D3DRS_STENCILENABLE,	true );
	device->SetRenderState( D3DRS_STENCILFUNC,		D3DCMP_EQUAL );
	device->SetRenderState( D3DRS_STENCILREF,		0x1 );
	device->SetRenderState( D3DRS_STENCILMASK,		0xffffffff );
	device->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
	device->SetRenderState( D3DRS_STENCILZFAIL,		D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_STENCILFAIL,		D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_STENCILPASS,		D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_ZENABLE,			false );
}
void DX9_GpuStencilClipSurface::End()
{
	device->SetRenderState( D3DRS_ZENABLE, true );
	device->SetRenderState( D3DRS_STENCILENABLE, false );
}
void DX9_GpuStencilClipSurface::Clear()
{
	Begin();
	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
	End();
}

#endif // USE_DX9_GPUAPI

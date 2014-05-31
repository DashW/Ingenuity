#ifdef USE_DX11_GPUAPI

#include "DX11Surfaces.h"

namespace Ingenuity {

DX11::TextureSurface::TextureSurface(DX11::Api * gpu, ID3D11Device * device, ID3D11DeviceContext * context,
	Format format, bool screenRelative, float width, float height) :
	DX11::DrawSurface(device, context),
	gpu(gpu),
	renderTargetView(0),
	depthStencilTexture(0),
	depthStencilView(0),
	texture(0),
	format(format),
	screenRelative(screenRelative),
	widthFactor(width),
	heightFactor(height)
{
	viewport = CD3D11_VIEWPORT(0.0f, 0.0f, width, height);

	OnResetDevice(gpu);
}
DX11::TextureSurface::~TextureSurface()
{
	OnLostDevice(gpu);
}

void DX11::TextureSurface::Begin()
{
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->RSSetViewports(1, &viewport);
}

void DX11::TextureSurface::End()
{
	gpu->ResetRenderTargets();
}

void DX11::TextureSurface::OnLostDevice(Gpu::Api * gpu)
{
	if(texture)
	{
		delete texture; texture = 0;
		renderTargetView->Release(); renderTargetView = 0;
		depthStencilTexture->Release(); depthStencilTexture = 0;
		depthStencilView->Release(); depthStencilView = 0;
	}
}

void DX11::TextureSurface::OnResetDevice(Gpu::Api * gpu)
{
	OnLostDevice(gpu);

	unsigned width = unsigned(viewport.Width);
	unsigned height = unsigned(viewport.Height);

	if(screenRelative)
	{
		gpu->GetBackbufferSize(width, height);
		width = unsigned(widthFactor * float(width));
		height = unsigned(heightFactor * float(height));
	}

	viewport = CD3D11_VIEWPORT(0.0f, 0.0f, float(width), float(height));

	static const DXGI_FORMAT SURFACE_FORMAT_TO_DXGI_FORMAT[Format_Total] =
	{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R16_FLOAT
	};

	DXGI_FORMAT dxgiFormat = SURFACE_FORMAT_TO_DXGI_FORMAT[format];

	ID3D11Texture2D * texture2D = 0;
	CD3D11_TEXTURE2D_DESC textureDesc(
		dxgiFormat,
		width,
		height,
		1,
		0, // MIP LEVELS - *MUST NEVER* BE GREATER THAN log2(width) OR log2(height) - OTHERWISE MYSTERIOUS CRASHES!!!
		// AS IT TURNS OUT, THIS MIGHT NOT BE THE CAUSE OF THE MYSTERIOUS CRASHES. uuurgh.
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	device->CreateTexture2D(&textureDesc, 0, &texture2D);

	ID3D11ShaderResourceView * shaderView;
	CD3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc(
		texture2D,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		textureDesc.Format,
		0,
		1);
	device->CreateShaderResourceView(texture2D, &shaderViewDesc, &shaderView);

	CD3D11_RENDER_TARGET_VIEW_DESC renderViewDesc(
		texture2D,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		textureDesc.Format);
	device->CreateRenderTargetView(texture2D, &renderViewDesc, &renderTargetView);

	texture = new DX11::Texture(texture2D, shaderView, textureDesc);

	CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		width,
		height,
		1,
		0,
		D3D11_BIND_DEPTH_STENCIL);
	device->CreateTexture2D(&depthStencilTextureDesc, 0, &depthStencilTexture);

	device->CreateDepthStencilView(
		depthStencilTexture,
		&CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D),
		&depthStencilView);

	Clear();
}

void DX11::TextureSurface::Clear(glm::vec4 color)
{
	// Clear the back buffer.
	context->ClearRenderTargetView(renderTargetView, (float*) &color);

	// Clear the depth buffer.
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

Gpu::DrawSurface::Type DX11::TextureSurface::GetSurfaceType()
{
	return screenRelative ? Gpu::DrawSurface::TypeFullscreenTexture : Gpu::DrawSurface::TypeTexture;
}

void DX11::StencilSurface::Begin()
{
	if(!stencilState)
	{
		D3D11_DEPTH_STENCIL_DESC mirrorDesc;
		mirrorDesc.DepthEnable = false;
		mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		mirrorDesc.DepthFunc = D3D11_COMPARISON_NEVER;
		mirrorDesc.StencilEnable = true;
		mirrorDesc.StencilReadMask = 0xff;
		mirrorDesc.StencilWriteMask = 0xff;

		mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// We are not rendering backfacing polygons, so these settings do not matter.
		mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		device->CreateDepthStencilState(&mirrorDesc, &stencilState);
	}
	context->OMSetDepthStencilState(stencilState, 1);
}
void DX11::StencilSurface::End()
{
	//device->SetRenderState( D3DRS_ZWRITEENABLE, true );
	context->OMSetDepthStencilState(0, 0);
}
void DX11::StencilSurface::Clear(glm::vec4 clearColor)
{
	ID3D11DepthStencilView * depthStencilView;
	context->OMGetRenderTargets(1, 0, &depthStencilView);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DX11::StencilClipSurface::Begin()
{
	if(!stencilClipState)
	{
		D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
		drawReflectionDesc.DepthEnable = false;
		drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
		drawReflectionDesc.StencilEnable = true;
		drawReflectionDesc.StencilReadMask = 0xff;
		drawReflectionDesc.StencilWriteMask = 0xff;

		drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

		// We are not rendering backfacing polygons, so these settings do not matter.
		drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

		device->CreateDepthStencilState(&drawReflectionDesc, &stencilClipState);
	}
	context->OMSetDepthStencilState(stencilClipState, 1);
}
void DX11::StencilClipSurface::End()
{
	//device->SetRenderState( D3DRS_ZENABLE, true );
	context->OMSetDepthStencilState(0, 0);
}
void DX11::StencilClipSurface::Clear(glm::vec4 color)
{
	ID3D11DepthStencilView * depthStencilView;
	context->OMGetRenderTargets(1, 0, &depthStencilView);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

} // namespace Ingenuity

#endif USE_DX11_GPUAPI

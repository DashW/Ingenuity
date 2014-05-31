#include "stdafx.h"

#ifdef USE_DX11_GPUAPI

#include "DX11Api.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <GeometricPrimitive.h>
#include <CommonStates.h>
#include <sstream>

using namespace DirectX;

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
DX11_GpuApi::DX11_GpuApi(HWND handle)
#else
DX11_GpuApi::DX11_GpuApi(Windows::UI::Core::CoreWindow^ window)
#endif
	:direct3Ddevice(0)
    ,direct3Dcontext(0)
	,dxgiSwapChain(0)
	,renderTargetView(0)
	,depthStencilView(0)
	,depthStencilBuffer(0)
	,wireframeState(0)
	,stencilState(0)
	,stencilClipState(0)
{
	UINT creationFlags = 0;

#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif	
	creationFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
 //       D3D_FEATURE_LEVEL_11_1,
//        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };
	
    D3D11CreateDevice(
        nullptr,                    
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,                    
        creationFlags,              
        featureLevels,              
        ARRAYSIZE(featureLevels),   
        D3D11_SDK_VERSION,          
        &direct3Ddevice,                    
        &featureLevel,           
        &direct3Dcontext);

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
    //swapChainDesc.Width = 0;                                     // use automatic sizing
    //swapChainDesc.Height = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // this is the most common swapchain format
	//swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	//swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    //swapChainDesc.Stereo = false; 
    swapChainDesc.SampleDesc.Count = 1;                          // don't use multi-sampling
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    //swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // we recommend using this swap effect for all applications
    swapChainDesc.Flags = 0;
	swapChainDesc.Windowed = true;
	swapChainDesc.OutputWindow = handle;

	IDXGIDevice* dxgiDevice = 0;
	direct3Ddevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	IDXGIAdapter* dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter),(void**)&dxgiAdapter);
	IDXGIFactory* dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory),(void**)&dxgiFactory);

	dxgiFactory->CreateSwapChain(
		dxgiDevice,
		&swapChainDesc,
		&dxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
    swapChainDesc.Width = 0;                                     // use automatic sizing
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // this is the most common swapchain format
    swapChainDesc.Stereo = false; 
    swapChainDesc.SampleDesc.Count = 1;                          // don't use multi-sampling
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;                               // use two buffers to enable flip effect
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
    swapChainDesc.Flags = 0;

	IDXGIDevice1* dxgiDevice = 0;
	direct3Ddevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
	IDXGIAdapter* dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter),(void**)&dxgiAdapter);
	IDXGIFactory2* dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2),(void**)&dxgiFactory);

	dxgiFactory->CreateSwapChainForCoreWindow(
		dxgiDevice,
		reinterpret_cast<IUnknown*>(window), 
		&swapChainDesc,
		nullptr,
		&dxgiSwapChain);
#endif

#ifdef USE_DIRECT2D
	
	D2D1_FACTORY_OPTIONS options = {};
#ifdef DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	ID2D1Factory* direct2Dfactory;
    D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory),
            &options,
            (void**) &direct2Dfactory
            );

	IDXGISurface* backBuffer;
	dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

//	D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(handle);

	//direct2Dfactory->
	direct2Dfactory->CreateDxgiSurfaceRenderTarget(backBuffer, &props, &direct2Dtarget);

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),(IUnknown**) &directWriteFactory);
	//delete backBuffer, direct2Dfactory;

#endif

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	SetScreenSize(0,0); // use HWND

	direct3Ddevice->CreateDepthStencilState(
		&CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT()),
		&depthStencilState);

	D3D11_SAMPLER_DESC samDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	direct3Ddevice->CreateSamplerState(&samDesc, &textureSampler);

	direct3Ddevice->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(DX11_VertexConstants),D3D11_BIND_CONSTANT_BUFFER),
		0,
		&vertexConstBuffer);
	direct3Ddevice->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(DX11_PixelConstants),D3D11_BIND_CONSTANT_BUFFER),
		0,
		&pixelConstBuffer);

	//FW1CreateFactory(FW1_VERSION,&fontFactory);

	spriteBatch = new SpriteBatch(direct3Dcontext);

	commonStates = new CommonStates(direct3Ddevice);

	clearColor[0] = 1.0f; clearColor[1] = 1.0f; clearColor[2] = 1.0f; clearColor[3] = 1.0f;

	initialised = true;
}
DX11_GpuApi::~DX11_GpuApi()
{
	DX11_VertApi::ReleaseVertices();

	if(textureSampler) textureSampler->Release();
	if(wireframeState) wireframeState->Release();
	if(stencilState) stencilState->Release();
	if(stencilClipState) stencilClipState->Release();
	if(vertexConstBuffer) vertexConstBuffer->Release();
	if(pixelConstBuffer) pixelConstBuffer->Release();
	if(depthStencilState) depthStencilState->Release();
	if(renderTargetView) renderTargetView->Release();
	if(depthStencilView) depthStencilView->Release();
	if(commonStates) delete commonStates;
	if(spriteBatch) delete spriteBatch;
	if(dxgiSwapChain) dxgiSwapChain->Release();

	if(direct3Dcontext) {
		direct3Dcontext->ClearState();
		direct3Dcontext->Flush();
		direct3Dcontext->Release();
	}
	if(direct3Ddevice) direct3Ddevice->Release();
}

//struct VShaderLoadedResponse : public FileMgr_Response
//{
//	DX11_GpuApi* parent;
//	VertexType type;
//	VShaderLoadedResponse(DX11_GpuApi* creator, VertexType vtxType)
//		: parent(creator), type(vtxType) {}
//	virtual void Respond() override
//	{
//		parent->BuildVertexShader(buffer,bufferLength,type);
//		closeOnComplete = true;
//	}
//};
//
//struct PShaderLoadedResponse : public FileMgr_Response
//{
//	DX11_GpuApi* parent;
//	VertexType type;
//	PShaderLoadedResponse(DX11_GpuApi* creator, VertexType vtxType)
//		: parent(creator), type(vtxType) {}
//	virtual void Respond() override
//	{
//		parent->BuildPixelShader(buffer,bufferLength,type);
//		closeOnComplete = true;
//	}
//};

void DX11_GpuApi::LoadShaders(FileMgr* fileManager)
{
}

void DX11_GpuApi::Clear() 
{
	direct3Dcontext->ClearRenderTargetView(renderTargetView,clearColor);
	direct3Dcontext->ClearDepthStencilView(depthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);
}
void DX11_GpuApi::BeginScene() 
{
	direct3Dcontext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

#ifdef USE_DIRECT2D
	direct2Dtarget->BeginDraw();
#endif

	spriteBatch->Begin();
}
void DX11_GpuApi::EndScene()
{
	spriteBatch->End();

#ifdef USE_DIRECT2D
	direct2Dtarget->EndDraw();
#endif

	direct3Dcontext->OMSetDepthStencilState(0,0);
	direct3Dcontext->RSSetState(0);
}
void DX11_GpuApi::Present()
{
	dxgiSwapChain->Present(0,0);
}

void DX11_GpuApi::DrawGpuText(GpuFont* font, LPCWSTR text, float x, float y, bool center) 
{
	if(!font) return;
	DX11_GpuFont* dx11font = static_cast<DX11_GpuFont*>(font);

#ifdef USE_DIRECT2D

	if(!font->pixelSpace)
	{
		x = ((float)(backBufferWidth/2)) + (x * backBufferHeight * 0.5f);
		y = ((float)(backBufferHeight/2)) + (y * backBufferHeight * 0.5f);
		const float scaleFactor = (((float)backBufferHeight)/((float)standardScreenHeight));
		direct2Dtarget->SetTransform(
			D2D1::Matrix3x2F::Scale(scaleFactor,scaleFactor,
			D2D1::Point2F(((float)(backBufferWidth/2)),((float)(backBufferHeight/2)))));
	}

	ID2D1SolidColorBrush* brush;

	direct2Dtarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush);

	D2D1_RECT_F layoutRect; 

	if(center)
	{
		dx11font->fontObject->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		dx11font->fontObject->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		layoutRect = D2D1::RectF( x - 1000.0f, y - 1000.0f, x + 1000.0f, y + 1000.0f);
	}
	else
	{
		dx11font->fontObject->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		dx11font->fontObject->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		layoutRect = D2D1::RectF( x, y, 
		static_cast<float>(backBufferWidth), 
		static_cast<float>(backBufferHeight));
	}

	direct2Dtarget->DrawText(
		text, 
		wcslen(text), 
		dx11font->fontObject, 
		&layoutRect,
		brush);

	brush->Release();
#else

	XMFLOAT2 position(0.0f,0.0f);
	XMFLOAT2 offset(0.0f,0.0f);
	if(center)
	{
		XMFLOAT2 widthHeight;
		XMStoreFloat2(&widthHeight,dx11font->fontObject->MeasureString(text));
		offset.x = widthHeight.x/2.0f; offset.y = widthHeight.y/2.0f;
	}

	float size;
	if(font->pixelSpace)
	{
		position.x = x; position.y = y;
		size = dx11font->fontSize;
	}
	else
	{
		position.x = ((float)(backBufferWidth/2)) + (x * backBufferHeight * 0.5f);
		position.y = ((float)(backBufferHeight/2)) + (y * backBufferHeight * 0.5f);
		size = dx11font->fontSize * (((float)backBufferHeight)/((float)standardScreenHeight));
	}

	XMVECTORF32 color = {font->colorR,font->colorG,font->colorB,font->colorA};

	//spriteBatch->Begin();//SpriteSortMode_Deferred,0,0,depthStencilState);
	dx11font->fontObject->DrawString(spriteBatch,text,position,color * font->colorA,
		0.0f,offset,size);
	//spriteBatch->End();

	//ID3D11BlendState blendState = 
	//direct3Dcontext->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
	//direct3Dcontext->OMSetDepthStencilState(0,0);
	//direct3Dcontext->RSSetState(0);
#endif
}
void DX11_GpuApi::DrawGpuMesh(GpuMesh* mesh, GpuCamera* camera, GpuLight** lights, 
							  unsigned numLights, bool wireFrame, GpuDrawBuffer* buffer) 
{
}
void DX11_GpuApi::DrawGpuIndexedMesh(GpuIndexedMesh* mesh, GpuCamera* camera, GpuLight** lights, 
									 unsigned numLights, bool wireFrame, GpuDrawBuffer* buffer) 
{
	DX11_GpuIndexedMesh* dx11mesh = static_cast<DX11_GpuIndexedMesh*>(mesh);

	if(shaderGroups[dx11mesh->vertexType].IsLoaded())
	{
		if(wireFrame)
		{
			if(!wireframeState)
			{
				D3D11_RASTERIZER_DESC rasterDesc;
				rasterDesc.AntialiasedLineEnable = false;
				rasterDesc.CullMode = D3D11_CULL_BACK;
				rasterDesc.DepthBias = 0;
				rasterDesc.DepthBiasClamp = 0.0f;
				rasterDesc.DepthClipEnable = true;
				rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
				rasterDesc.FrontCounterClockwise = false;
				rasterDesc.MultisampleEnable = false;
				rasterDesc.ScissorEnable = false;
				rasterDesc.SlopeScaledDepthBias = 0.0f;
				direct3Ddevice->CreateRasterizerState(&rasterDesc,&wireframeState);
			}
			direct3Dcontext->RSSetState(wireframeState);
		}

		direct3Dcontext->IASetInputLayout(DX11_VertApi::GetInputLayout(dx11mesh->vertexType));
		direct3Dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = dx11mesh->vertexSize;
		UINT offset = 0;
		direct3Dcontext->IASetVertexBuffers(0, 1, &dx11mesh->vertexBuffer, &stride, &offset);
		direct3Dcontext->IASetIndexBuffer(dx11mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX w = XMMatrixIdentity();
		w *= XMMatrixRotationRollPitchYaw(dx11mesh->rotationX,dx11mesh->rotationY,dx11mesh->rotationZ);
		w *= XMMatrixTranslation(dx11mesh->positionX,dx11mesh->positionY,dx11mesh->positionZ);
		XMMATRIX v = XMLoadFloat4x4(&view);
		XMMATRIX p = XMLoadFloat4x4(&projection);
		XMMATRIX worldViewProj = w*v*p;

		switch(dx11mesh->vertexType)
		{
		case VertexType_PosNorTex:
			if(mesh->texture)
			{
				ID3D11SamplerState* samplerState = commonStates->LinearWrap();
				direct3Dcontext->PSSetSamplers(0, 1, &samplerState);

				DX11_GpuTexture* texture = static_cast<DX11_GpuTexture*>(mesh->texture);
				direct3Dcontext->PSSetShaderResources(0,1,&texture->shaderView);
			}
			// And all of the following...
		case VertexType_PosNor:
			if(numLights > 0)
			{
				pixelConstData.ambient = 0.0f;
				if(lights[0]->GetType() == GpuLightType_Directional)
				{
					GpuDirectionalLight* dirLight = static_cast<GpuDirectionalLight*>(lights[0]);
					XMVECTOR lightDirVec = XMLoadFloat3(&XMFLOAT3(dirLight->u,dirLight->v,dirLight->w));
					XMStoreFloat3(&pixelConstData.lightPosition, XMVectorScale(lightDirVec,10000.f));
					pixelConstData.lightColor = XMFLOAT4(dirLight->diffuseR,dirLight->diffuseG,dirLight->diffuseB,1.0f);
					pixelConstData.cameraPosition = XMFLOAT3(camera->x,camera->y,camera->z);
					pixelConstData.specularPower = 16.0f;
					pixelConstData.spotPower = 0.0f;
					pixelConstData.spotDirection = XMFLOAT3(0.0f,0.0f,0.0f);
				}
				if(lights[0]->GetType() == GpuLightType_Point)
				{
					GpuPointLight* pointLight = static_cast<GpuPointLight*>(lights[0]);
					pixelConstData.lightPosition = XMFLOAT3(pointLight->x,pointLight->y,pointLight->z);
					pixelConstData.lightColor = XMFLOAT4(pointLight->diffuseR,pointLight->diffuseG,pointLight->diffuseB,1.0f);
					pixelConstData.cameraPosition = XMFLOAT3(camera->x,camera->y,camera->z);
					pixelConstData.specularPower = 16.0f;
					pixelConstData.spotPower = 0.0f;
					pixelConstData.spotDirection = XMFLOAT3(0.0f,0.0f,0.0f);
				}
				if(lights[0]->GetType() == GpuLightType_Spot)
				{
					GpuSpotLight* spotLight = static_cast<GpuSpotLight*>(lights[0]);
					pixelConstData.lightPosition = XMFLOAT3(spotLight->x,spotLight->y,spotLight->z);
					pixelConstData.lightColor = XMFLOAT4(spotLight->diffuseR,spotLight->diffuseG,spotLight->diffuseB,1.0f);
					pixelConstData.cameraPosition = XMFLOAT3(camera->x,camera->y,camera->z);
					pixelConstData.specularPower = 16.0f;
					pixelConstData.spotPower = spotLight->power;
					XMStoreFloat3(&pixelConstData.spotDirection,XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(spotLight->u,spotLight->v,spotLight->w))));
				}
				direct3Dcontext->UpdateSubresource(pixelConstBuffer,0,0,&pixelConstData,0,0);
				direct3Dcontext->PSSetConstantBuffers(0,1,&pixelConstBuffer);
			}
			else
			{
				pixelConstData.lightPosition = XMFLOAT3(0.0f,0.0f,0.0f);
				pixelConstData.lightColor = XMFLOAT4(1.0f,1.0f,1.0f,1.0f);
				pixelConstData.cameraPosition = XMFLOAT3(camera->x,camera->y,camera->z);
				pixelConstData.spotPower = 0.0f;
				pixelConstData.specularPower = FLT_MAX;
				pixelConstData.spotDirection = XMFLOAT3(0.0f,0.0f,0.0f);
				pixelConstData.ambient = 1.0f;
				direct3Dcontext->UpdateSubresource(pixelConstBuffer,0,0,&pixelConstData,0,0);
				direct3Dcontext->PSSetConstantBuffers(0,1,&pixelConstBuffer);
			}

			XMStoreFloat4x4(&vertexConstData.world,w);
			XMStoreFloat4x4(&vertexConstData.worldInverseTranspose,XMMatrixInverse(0,XMMatrixTranspose(w)));
			vertexConstData.materialColor = XMFLOAT4(1.0f,0.0f,0.0f,1.0f);

			// And all of the following...
		default: //VertexType_PosCol
			XMStoreFloat4x4(&vertexConstData.worldViewProjection,worldViewProj);// was XMMatrixTranspose(worldViewProj); //fixed by CG	

			direct3Dcontext->UpdateSubresource(vertexConstBuffer,0,0,&vertexConstData,0,0);
			direct3Dcontext->VSSetConstantBuffers(0,1,&vertexConstBuffer);

			direct3Dcontext->VSSetShader(shaderGroups[dx11mesh->vertexType].vertexShader,0,0);
			direct3Dcontext->PSSetShader(shaderGroups[dx11mesh->vertexType].pixelShader,0,0);
		}

		SetBufferState(buffer);

		direct3Dcontext->DrawIndexed(dx11mesh->numTriangles * 3, 0, 0);

		RestoreBufferState(buffer);
	}
}
void DX11_GpuApi::DrawGpuSprite(GpuSprite* sprite) 
{
	if(!sprite->texture) return;

	DX11_GpuTexture* dx11tex = static_cast<DX11_GpuTexture*>(sprite->texture);
	//ID3D11BlendState* blendState;
	//if(sprite->brightAsAlpha)
	//	blendState = commonStates->Additive();
	//else
	//	blendState = commonStates->AlphaBlend();

	//D3D11_TEXTURE2D_DESC desc;
	//ID3D11Texture2D* texture2D;
	//dx11tex->texture->QueryInterface(__uuidof(ID3D11Texture2D),(void**)&texture2D);
	//texture2D->GetDesc(&desc);
	RECT sampleRect = {0,0,(long)sprite->transformCenterX*2,(long)sprite->transformCenterY*2};

	XMFLOAT2 position;
	float w = static_cast<float>(backBufferWidth/2);
	float h = static_cast<float>(backBufferHeight/2);
	if(sprite->pixelSpace)
	{
		position.x = sprite->positionX; position.y = sprite->positionY;
	}
	else
	{
		position.x = w + (sprite->positionX * h);
		position.y = h + (sprite->positionY * h);
	}

	//direct3Dcontext->OMSetDepthStencilState(commonStates->DepthRead(),0);

	spriteBatch->Draw(dx11tex->shaderView, position,
		&sampleRect,Colors::White,0.0f,
		XMFLOAT2(sprite->transformCenterX,sprite->transformCenterY),
		sprite->pixelSpace ? sprite->size : sprite->size * 
		(((float)backBufferHeight)/((float)standardScreenHeight)),
		DirectX::SpriteEffects_None,sprite->positionZ);

	//texture2D->Release();

	//direct3Dcontext->OMSetDepthStencilState(0,0);
	//direct3Dcontext->RSSetState(0);
}
void DX11_GpuApi::DrawGpuScene(GpuScene* scene)
{
}

void DX11_GpuApi::LookTransform(
	float x,float y,float z,
	float tx,float ty,float tz,
	float ux,float uy,float uz) 
{
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorSet(tx, ty, tz, 1.0f);
	XMVECTOR up     = XMVectorSet(ux, uy, uz, 0.0f);

	XMMATRIX v = XMMatrixLookAtLH(pos, target, up);

	XMStoreFloat4x4(&view,v);
}
void DX11_GpuApi::PerspectiveTransform(float fov_y, float aspect, float nearz, float farz) 
{
	XMMATRIX p = XMMatrixPerspectiveFovLH(fov_y, aspect, nearz, farz);

	XMStoreFloat4x4(&projection, p);
}

GpuFont* DX11_GpuApi::CreateGpuFont(int height, LPCWSTR facename, GpuFontStyle style) 
{
#ifdef USE_DIRECT2D
	DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL;
	DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL;

	switch(style)
	{
	case GpuFontStyle_Bold:
		fontWeight = DWRITE_FONT_WEIGHT_BOLD;
		break;
	case GpuFontStyle_Italic:
		fontStyle = DWRITE_FONT_STYLE_ITALIC;
	}

	IDWriteTextFormat* textFormat;
	directWriteFactory->CreateTextFormat(facename,0,fontWeight,fontStyle,
		DWRITE_FONT_STRETCH_NORMAL,(float) height,L"en-US",&textFormat);
	return new DX11_GpuFont(textFormat);
#else
	std::wstringstream ss; 
	ss << facename;
	switch(style)
	{
	case GpuFontStyle_Bold:
		ss << L"B";
		break;
	case GpuFontStyle_Italic:
		ss << L"I";
		break;
	default:
		break;
	}
	ss << L".spritefont";

	WIN32_FIND_DATA findData = {0};
	FindFirstFileEx( ss.str().c_str(), 
					 FindExInfoStandard, 
					 &findData, 
					 FindExSearchNameMatch, 
					 NULL, 
					 0 );
	if(findData.nFileSizeLow)
	{
		SpriteFont* font = new SpriteFont(direct3Ddevice,ss.str().c_str());
		return new DX11_GpuFont(font,facename,((float)height)/(95.f));
	}
	//ERROR Sprite font not found
	return 0;
#endif
}
GpuTexture* DX11_GpuApi::CreateGpuTextureFromFile(LPCWSTR path) 
{
	DX11_GpuTexture* texture = new DX11_GpuTexture();
	unsigned length = wcslen(path);
	if(_wcsicmp(L".dds",&path[length - 4]) == 0)
		CreateDDSTextureFromFile(direct3Ddevice,path,&texture->texture,&texture->shaderView);
	else
		CreateWICTextureFromFile(direct3Ddevice,direct3Dcontext,path,&texture->texture,&texture->shaderView);
	return texture;
}
GpuMesh* DX11_GpuApi::CreateGpuMesh(
	unsigned numVertices, unsigned size, void* vertexBufferData, unsigned numTriangles) 
{
	return 0;
}
GpuMesh* DX11_GpuApi::CreateGpuMesh(VertexBuffer* buffer, unsigned numTriangles)
{
	return 0;
}
GpuIndexedMesh* DX11_GpuApi::CreateGpuIndexedMesh(
	unsigned numVertices, unsigned vertexDataSize, void* vertexData,
	unsigned numTriangles, unsigned indexDataSize, unsigned* indexData, VertexType type) 
{
	DX11_GpuIndexedMesh* createdMesh = new DX11_GpuIndexedMesh();

	D3D11_BUFFER_DESC vbdesc;
	vbdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbdesc.ByteWidth = vertexDataSize;
	vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbdesc.CPUAccessFlags = 0;
	vbdesc.MiscFlags = 0;
	vbdesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vbInitData;
	vbInitData.pSysMem = vertexData;
	direct3Ddevice->CreateBuffer(&vbdesc,&vbInitData,&createdMesh->vertexBuffer);

	D3D11_BUFFER_DESC ibdesc;
	ibdesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibdesc.ByteWidth = indexDataSize;
	ibdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibdesc.CPUAccessFlags = 0;
	ibdesc.MiscFlags = 0;
	ibdesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA ibInitData;
	ibInitData.pSysMem = indexData;
	direct3Ddevice->CreateBuffer(&ibdesc,&ibInitData,&createdMesh->indexBuffer);

	createdMesh->inputLayout = DX11_VertApi::GetInputLayout(type);
	createdMesh->vertexSize = DX11_VertApi::GetSize(type);
	createdMesh->numVertices = numVertices;
	createdMesh->numTriangles = numTriangles;
	createdMesh->vertexType = type;

	return createdMesh;
}
GpuIndexedMesh* DX11_GpuApi::CreateGpuIndexedMesh(VertexBuffer* buffer, unsigned numTriangles, 
		unsigned indexDataSize, unsigned* indexData)
{
	return CreateGpuIndexedMesh(
		buffer->GetLength(), 
		buffer->GetLength() * buffer->GetElementSize(),
		buffer->GetData(),
		numTriangles,
		indexDataSize,
		indexData,
		buffer->GetVertexType());
}

GpuIndexedMesh* DX11_GpuApi::CreateTeapot() 
{
	//DX11_GpuIndexedMesh* mesh = new DX11_GpuIndexedMesh();
	//std::unique_ptr<GeometricPrimitive> teapot = GeometricPrimitive::CreateTeapot(direct3Dcontext);
	//teapot->
	
	return 0;
}
GpuIndexedMesh* DX11_GpuApi::CreateCube()
{
	return 0;
}
GpuIndexedMesh* DX11_GpuApi::CreateCylinder(float radius, float length, unsigned slices, unsigned stacks, bool texCoords)
{
	VertexBuffer* buffer = BuildVertexBufferCylinder(length, radius, slices, stacks, texCoords, false);
	unsigned indexLength;
	unsigned* indices = BuildIndexBufferCylinder(slices, stacks, &indexLength);
	return CreateGpuIndexedMesh(buffer, indexLength/3, indexLength * sizeof(unsigned), indices);
	return 0;
}
GpuIndexedMesh* DX11_GpuApi::CreateSphere(float radius, unsigned slices, unsigned stacks, bool texCoords)
{
	VertexBuffer* buffer = BuildVertexBufferCylinder(radius*2.0f, radius, slices, stacks, texCoords, true);
	unsigned indexLength;
	unsigned* indices = BuildIndexBufferCylinder(slices, stacks, &indexLength);
	return CreateGpuIndexedMesh(buffer, indexLength/3, indexLength * sizeof(unsigned), indices);
	//return 0;
}
GpuIndexedMesh* DX11_GpuApi::CreateGrid(float width, float depth, unsigned gridcolumns, 
										unsigned gridrows, GpuRect* textureRect)
{
	unsigned numVertices = gridcolumns * gridrows;

	DX11VertexBuffer_PosNor* vertexBuffer = new DX11VertexBuffer_PosNor(numVertices);
	DX11VertexBuffer_PosNorTex* vertexBufferTex = new DX11VertexBuffer_PosNorTex(numVertices);

	if(textureRect)
	{
		for(unsigned i = 0; i < gridrows; i++) {
			for(unsigned j = 0; j < gridcolumns; j++) {
				float x = (width * -.5f) + (j * (width/(gridcolumns - 1)));
				float z = (depth * -.5f) + (i * (depth/(gridrows - 1)));
				const float texRectWidth = textureRect->right - textureRect->left;
				const float texRectHeight = textureRect->bottom - textureRect->top;
				float tx = textureRect->left + (j * (texRectWidth/(gridcolumns - 1)));
				float ty = textureRect->top + (i * (texRectHeight/(gridrows - 1)));
				vertexBufferTex->Insert((gridcolumns * i) + j, x, 0.0f, z, 0.0f, 1.0f, 0.0f,tx,ty);
			}
		}
	}
	else
	{
		for(unsigned i = 0; i < gridrows; i++) {
			for(unsigned j = 0; j < gridcolumns; j++) {
				float x = (width * -.5f) + (j * (width/(gridcolumns - 1)));
				float z = (depth * -.5f) + (i * (depth/(gridrows - 1)));
				vertexBuffer->Insert((gridcolumns * i) + j, x, 0.0f, z, 0.0f, 1.0f, 0.0f);
			}
		}
	}

	unsigned numTriangles = (gridcolumns - 1) * (gridrows - 1) * 2;

	unsigned* k = new unsigned(numTriangles * 3);

	int index = 0;

	for(unsigned i = 0; i < (gridrows - 1); i++) {
		for(unsigned j = 0; j < (gridcolumns - 1); j++) {
			k[index++] = j + (i * gridcolumns);
			k[index++] = j + ((i+1) * gridcolumns);
			k[index++] = j + 1 + (i * gridcolumns);

			k[index++] = j + 1 + (i * gridcolumns);
			k[index++] = j + ((i+1) * gridcolumns);
			k[index++] = j + 1 + ((i+1) * gridcolumns);
		}
	}

	GpuIndexedMesh* createdMesh;

	if(textureRect)
		createdMesh = CreateGpuIndexedMesh(vertexBufferTex,numTriangles,sizeof(unsigned)*numTriangles*3,k);
	else
		createdMesh = CreateGpuIndexedMesh(vertexBuffer,numTriangles,sizeof(unsigned)*numTriangles*3,k);

	delete vertexBuffer,vertexBufferTex,k;

	return createdMesh;
}

void DX11_GpuApi::SetClearColor(float r, float g, float b)
{
	clearColor[0] = r; clearColor[1] = g; clearColor[2] = b;
}

void DX11_GpuApi::SetScreenSize(int width, int height) 
{
	if(renderTargetView) renderTargetView->Release();
	if(depthStencilView) depthStencilView->Release();

	if(width > 0 && height > 0)
		dxgiSwapChain->ResizeBuffers(2,(unsigned)width,(unsigned)height,DXGI_FORMAT_B8G8R8A8_UNORM,0);

	ID3D11Texture2D* backBuffer;
	dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	direct3Ddevice->CreateRenderTargetView(backBuffer,0,&renderTargetView);

	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBuffer->GetDesc(&backBufferDesc);

	backBuffer->Release();

	backBufferWidth = backBufferDesc.Width;
	backBufferHeight = backBufferDesc.Height;

	CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		backBufferWidth,
		backBufferHeight,
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL);

	ID3D11Texture2D* depthStencilBuffer;
	direct3Ddevice->CreateTexture2D(
		&depthStencilTextureDesc,
		0,
		&depthStencilBuffer);

	direct3Ddevice->CreateDepthStencilView(
		depthStencilBuffer,
		&CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D),
		&depthStencilView);

	direct3Dcontext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	depthStencilBuffer->Release();

	CD3D11_VIEWPORT viewPort(
		0.0f,
		0.0f,
		static_cast<float>(backBufferWidth),
		static_cast<float>(backBufferHeight));

	direct3Dcontext->RSSetViewports(1,&viewPort);
}

void DX11_GpuApi::SetBufferState(GpuDrawBuffer* buffer)
{
	if(buffer != 0)
	{
		switch (buffer->GetSpecial())
		{
		case GpuSpecialBuffer_Stencil:
			if(!stencilState)
			{
				D3D11_DEPTH_STENCIL_DESC mirrorDesc;
				mirrorDesc.DepthEnable      = false;
				mirrorDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;
				mirrorDesc.DepthFunc        = D3D11_COMPARISON_NEVER; 
				mirrorDesc.StencilEnable    = true;
				mirrorDesc.StencilReadMask  = 0xff;
				mirrorDesc.StencilWriteMask = 0xff;

				mirrorDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
				mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				mirrorDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
				mirrorDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

				// We are not rendering backfacing polygons, so these settings do not matter.
				mirrorDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
				mirrorDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
				mirrorDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
				mirrorDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

				direct3Ddevice->CreateDepthStencilState(&mirrorDesc, &stencilState);
			}
			direct3Dcontext->OMSetDepthStencilState(stencilState,1);
			break;
		case GpuSpecialBuffer_StencilClip:
			if(!stencilClipState)
			{
				D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
				drawReflectionDesc.DepthEnable      = false;
				drawReflectionDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
				drawReflectionDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
				drawReflectionDesc.StencilEnable    = true;
				drawReflectionDesc.StencilReadMask  = 0xff;
				drawReflectionDesc.StencilWriteMask = 0xff;

				drawReflectionDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
				drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				drawReflectionDesc.FrontFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

				// We are not rendering backfacing polygons, so these settings do not matter.
				drawReflectionDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
				drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
				drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
				drawReflectionDesc.BackFace.StencilFunc   = D3D11_COMPARISON_EQUAL;

				direct3Ddevice->CreateDepthStencilState(&drawReflectionDesc, &stencilClipState);
			}
			direct3Dcontext->OMSetDepthStencilState(stencilClipState,1);
			break;
		default:
			break;
		}
	}
}

void DX11_GpuApi::RestoreBufferState(GpuDrawBuffer* buffer)
{
	if(buffer != 0)
	{
		switch (buffer->GetSpecial())
		{
		case GpuSpecialBuffer_Stencil:
		case GpuSpecialBuffer_StencilClip:
			direct3Dcontext->OMSetDepthStencilState(0, 0);
			break;
		default:
			break;
		}
	}
}

bool DX11_GpuApi::isDeviceLost() 
{
	return false;
}

void DX11_GpuApi::BuildVertexShader(void* buffer, unsigned bufferLength, VertexType vertexType)
{
	switch (vertexType)
	{
	case VertexType_Pos:
		break;
	case VertexType_PosCol:
		direct3Ddevice->CreateVertexShader(buffer,bufferLength,0,&shaderGroups[vertexType].vertexShader);
		DX11Vertex_PosCol::initInputLayout(direct3Ddevice,buffer,bufferLength);
		break;
	case VertexType_PosNor:
		direct3Ddevice->CreateVertexShader(buffer,bufferLength,0,&shaderGroups[vertexType].vertexShader);
		DX11Vertex_PosNor::initInputLayout(direct3Ddevice,buffer,bufferLength);
		break;
	case VertexType_PosTex:
		break;
	case VertexType_PosNorTex:
		direct3Ddevice->CreateVertexShader(buffer,bufferLength,0,&shaderGroups[vertexType].vertexShader);
		DX11Vertex_PosNorTex::initInputLayout(direct3Ddevice,buffer,bufferLength);
		break;
	default:
		break;
	}
}

void DX11_GpuApi::BuildPixelShader(void* buffer, unsigned bufferLength, VertexType vertexType)
{
	switch (vertexType)
	{
	case VertexType_Pos:
		break;
	case VertexType_PosCol:
		direct3Ddevice->CreatePixelShader(buffer,bufferLength,0,&shaderGroups[vertexType].pixelShader);
		break;
	case VertexType_PosNor:
		direct3Ddevice->CreatePixelShader(buffer,bufferLength,0,&shaderGroups[vertexType].pixelShader);
		break;
	case VertexType_PosTex:
		break;
	case VertexType_PosNorTex:
		direct3Ddevice->CreatePixelShader(buffer,bufferLength,0,&shaderGroups[vertexType].pixelShader);
		break;
	default:
		break;
	}
}

//LPCWSTR DX11_GpuApi::GetUnicodeString(LPCSTR c_string)
//{
//	int a = lstrlenA(c_string);
//	BSTR unicodestr = SysAllocStringLen(NULL, a);
//	::MultiByteToWideChar(CP_ACP, 0, c_string, a, unicodestr, a);
//	return unicodestr;
//}

DX11_GpuFont::~DX11_GpuFont()
{
	if(fontObject) delete fontObject;
}

DX11_GpuIndexedMesh::~DX11_GpuIndexedMesh()
{
	if(vertexBuffer) vertexBuffer->Release(); vertexBuffer = 0;
	if(indexBuffer) indexBuffer->Release(); indexBuffer = 0;
}

DX11_GpuTexture::~DX11_GpuTexture()
{
	if(shaderView) shaderView->Release();
	if(texture) texture->Release();
}

VertexBuffer* DX11_GpuApi::BuildVertexBufferCylinder(const float inputHeight,
	const float inputRadius, const unsigned sphereSectors, const unsigned sphereSlices, 
	const bool texCoords, const bool sphere)
{
	unsigned numVertices = ((sphereSectors+1) * (sphereSlices + 1)) + 2;
	DX11VertexBuffer_PosNor* b = 0;
	DX11VertexBuffer_PosNorTex* bTex = 0;
	if(texCoords)
	{	
		bTex = new DX11VertexBuffer_PosNorTex(numVertices);
		bTex->Insert(0, -0.0f, -0.0f, -inputHeight/2.0f, -0.0f, -0.0f, -1.0f, 0.0f, 0.0f);
		bTex->Insert(numVertices - 1, -0.0f, -0.0f, inputHeight/2.0f, -0.0f, -0.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		b = new DX11VertexBuffer_PosNor(numVertices);
		b->Insert(0, -0.0f, -0.0f, -inputHeight/2.0f, -0.0f, -0.0f, -1.0f);
		b->Insert(numVertices - 1, -0.0f, -0.0f, inputHeight/2.0f, -0.0f, -0.0f, 1.0f);
	}
	
	for(unsigned i = 0; i < (sphereSlices + 1); i++)
	{
		const float depth = ((-1.0f + 
			(sphere ? (2.0f / (sphereSlices + 2)) * (i + 1) : (2.0f / (sphereSlices)) * i)) 
			* (inputHeight/2.0f));
		float sectorRadius;
		if(sphere)
			sectorRadius = sqrtf( powf(inputRadius,2.0f) - powf(depth,2.0f) );
		else
			sectorRadius = inputRadius;
		for(unsigned j = 0; j < (sphereSectors+1); j++)
		{
			float x = sin((2 * XM_PI / (sphereSectors)) * j) * sectorRadius;
			float y = cos((2 * XM_PI / (sphereSectors)) * j) * sectorRadius;
			float normalx = x/inputRadius;
			float normaly = y/inputRadius;
			float normalz = sphere ? depth/inputRadius : 0.0f;
			if(texCoords)
			{
				bTex->Insert(((sphereSectors+1) * i) + j + 1, x, y, depth, normalx, normaly, normalz, 
					((float)j)/((float)(sphereSectors)), ((float)i)/((float)sphereSlices));
			}
			else
			{
				b->Insert(((sphereSectors+1) * i) + j + 1, x, y, depth, normalx, normaly, normalz);
			}
		}
	}
	if(texCoords)
		return bTex;
	else
		return b;
}

unsigned* DX11_GpuApi::BuildIndexBufferCylinder(unsigned sphereSectors, unsigned sphereSlices, unsigned* length)
{
	unsigned numPrimitives = (sphereSectors+1) * 2 + (sphereSlices * (sphereSectors+1) * 2);
	unsigned numVertices = ((sphereSectors+1) * (sphereSlices + 1)) + 2;

	*length = numPrimitives * 3;
	unsigned* k = new unsigned[*length];
	int index = 0;

	for(unsigned i = 0; i < sphereSectors; i++)
	{
		// Bottom
		k[index++] = 0; 
		k[index++] = i + 1; 
		k[index++] = ((i+1)) + 1;

		// Quads
		for(unsigned j = 0; j < sphereSlices; j++)
		{
			k[index++] = (i+1) + j * (sphereSectors+1);                            //TOPLEFT
			k[index++] = (i+1) + (j+1) * (sphereSectors+1);                        //BOTTOMLEFT
			k[index++] = ((i+1)) + 1 + (j * (sphereSectors+1));      //TOPRIGHT

			k[index++] = ((i+1)) + 1 + (j * (sphereSectors+1));      //TOPRIGHT
			k[index++] = (i+1) + ((j+1) * (sphereSectors+1));                      //BOTTOMLEFT
			k[index++] = ((i+1)) + 1 + ((j+1) * (sphereSectors+1));  //BOTTOMRIGHT
		}

		//Top
		k[index++] = numVertices - 1;
		k[index++] = (numVertices - sphereSectors) + i - 1;
		k[index++] = (numVertices - sphereSectors) + i - 2;
	}

	return k;
}

#endif
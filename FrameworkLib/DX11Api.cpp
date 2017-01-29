#include "stdafx.h"

#ifdef USE_DX11_GPUAPI

#include "DX11Api.h"
#include "DX11ShaderLoader.h"
#include "DX11Surfaces.h"
#include "GeoBuilder.h"
#include "PlatformApi.h"
#include "ShaderParser.h"
#include "tinyxml2.h"
#include "Win32Window.h"

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#include <DirectXMath.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <CommonStates.h>
#include <GeometricPrimitive.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>
#include <string>

using namespace DirectX;

namespace Ingenuity {

float secsPerCnt;

DX11::Api::Api(PlatformWindow * window) :
	direct3Ddevice(0),
    direct3Dcontext(0),
	mainSwapChain(0),
	mainDrawSurface(0),
	wireframeState(0),
	samplerMgr(0),
	spriteBatch(0),
	commonStates(0),
	currentDepthMode(Gpu::DepthMode_ReadWrite),
	currentDepthStencilKey(-1),
	texCopyShader(0),
	texShaderQuad(0),
	texShaderLayout(0),
	texVertexShader(0),
	texVertexShaderRequested(false),
	currFrame(0)
{
	UINT creationFlags = 0;

#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif	
	creationFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
		D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };
	
    D3D11CreateDevice(
        0,                    
        D3D_DRIVER_TYPE_HARDWARE,
        0,                    
        creationFlags,              
        featureLevels,              
        ARRAYSIZE(featureLevels),   
        D3D11_SDK_VERSION,          
        &direct3Ddevice,                    
        &featureLevel,           
        &direct3Dcontext);

#ifdef _DEBUG
	ID3D11Debug *d3dDebug = nullptr;
	if(SUCCEEDED(direct3Ddevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		ID3D11InfoQueue *d3dInfoQueue = nullptr;
		if(SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET
				// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter;
			memset(&filter, 0, sizeof(filter));
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			d3dInfoQueue->Release();
		}
		d3dDebug->Release();
	}
#endif

	OnWindowCreated(window);

	ResizeWindowResources(window, 0, 0);

	CD3D11_RASTERIZER_DESC rasterDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, false, 0, 0.0f, 0.0f, true, false, false, false);
	direct3Ddevice->CreateRasterizerState(&rasterDesc, &defaultRasterState);

	samplerMgr = new SamplerMgr(direct3Ddevice, direct3Dcontext);

	spriteBatch = new SpriteBatch(direct3Dcontext);

	//LocalMesh * quad = GeoBuilder().BuildRect(0.0f, 0.0f, 1.0f, 1.0f, true);
	//texShaderQuad = static_cast<DX11::Mesh*>(quad->GpuOnly(this));

	VertexBuffer<Vertex_PosTex> triVertices(3);
	triVertices.Set(0, Vertex_PosTex(0.0f, 0.0f, 0.0f, 0.0f, 1.0f));
	triVertices.Set(1, Vertex_PosTex(2.0f, 0.0f, 0.0f, 2.0f, 1.0f));
	triVertices.Set(2, Vertex_PosTex(0.0f, 2.0f, 0.0f, 0.0f,-1.0f));
	unsigned triIndices[3] = { 0, 1, 2 };
	texShaderQuad = static_cast<DX11::Mesh*>(CreateGpuMesh(3, triVertices.GetData(), 1, triIndices, VertexType_PosTex));

	commonStates = new CommonStates(direct3Ddevice);

	clearColor[0] = 0.0f; clearColor[1] = 0.0f; clearColor[2] = 0.0f; clearColor[3] = 1.0f;

	for(unsigned i = 0; i < Gpu::BlendMode_Count; ++i)
	{
		blendStates[i] = 0;
	}

	//stencilSurface = new DX11::StencilSurface(direct3Ddevice,direct3Dcontext);
	//stencilClipSurface = new DX11::StencilClipSurface(direct3Ddevice,direct3Dcontext);

	//initialised = true;

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	secsPerCnt = 1.0f / (float)cntsPerSec;
}

DX11::Api::~Api()
{
	WindowSwapChainMap::iterator swapIt = windowSwapChains.begin();
	for( ; swapIt != windowSwapChains.end(); ++swapIt)
	{
		swapIt->second->Release();
		delete windowDrawSurfaces[swapIt->first];
	}

	DepthStencilBank::iterator dsIt = depthStencilStates.begin();
	for(; dsIt != depthStencilStates.end(); ++dsIt)
	{
		dsIt->second->Release();
	}

	for(unsigned i = 0; i < Gpu::BlendMode_Count; ++i)
	{
		if(blendStates[i]) blendStates[i]->Release();
	}

	CoUninitialize();

	if(texVertexShader) texVertexShader->Release();
	if(texShaderLayout) texShaderLayout->Release();
	if(texShaderQuad) delete texShaderQuad;
	if(wireframeState) wireframeState->Release();
	if(defaultRasterState) defaultRasterState->Release();
	//if(stencilSurface) delete stencilSurface;
	//if(stencilClipSurface) delete stencilClipSurface;
	if(commonStates) delete commonStates;
	if(spriteBatch) delete spriteBatch;
	if(samplerMgr) delete samplerMgr;
	if(baseEffect) delete baseEffect;

	if(direct3Dcontext) {
		direct3Dcontext->ClearState();
		direct3Dcontext->Flush();
		direct3Dcontext->Release();
	}
	if(direct3Ddevice) direct3Ddevice->Release();

//#ifdef _DEBUG
//	IDXGIDebug * dxgiDebug;
//	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&dxgiDebug);
//	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
//	dxgiDebug->Release();
//#endif
}

const D3D11_INPUT_ELEMENT_DESC vtxPos[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
const D3D11_INPUT_ELEMENT_DESC vtxPosCol[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
const D3D11_INPUT_ELEMENT_DESC vtxPosNor[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
const D3D11_INPUT_ELEMENT_DESC vtxPosTex[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
const D3D11_INPUT_ELEMENT_DESC vtxPosNorTex[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
const D3D11_INPUT_ELEMENT_DESC vtxPosNorTanTex[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC instPos[] = {
	{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
};
const D3D11_INPUT_ELEMENT_DESC instPosCol[] = {
	{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	{"COLOR", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1}
};
const D3D11_INPUT_ELEMENT_DESC instPosSca[] = {
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};
const D3D11_INPUT_ELEMENT_DESC instPosRotSca[] = {
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR", 2, DXGI_FORMAT_R32G32B32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

const D3D11_INPUT_ELEMENT_DESC * DX11::Api::vertexDescs[VertexType_Count] =
{
	vtxPos,
	vtxPosCol,
	vtxPosNor,
	vtxPosTex,
	vtxPosNorTex,
	vtxPosNorTanTex
};
const unsigned DX11::Api::vertexDescSizes[VertexType_Count] =
{
	1,
	2,
	2,
	2,
	3,
	4
};
const D3D11_INPUT_ELEMENT_DESC * DX11::Api::instanceDescs[InstanceType_Count] =
{
	0,
	instPos,
	instPosCol,
	instPosSca,
	instPosRotSca
};
const unsigned DX11::Api::instanceDescSizes[InstanceType_Count] =
{
	0,
	1,
	2,
	2,
	3
};

bool DX11::Api::SetDepthStencilState(DX11::DrawSurface * surface)
{
	unsigned depthStencilKey = 0;

	depthStencilKey |= currentDepthMode;

	// TODO: Add stencil state here (should be stored in the Surface)

	if(depthStencilStates.count(depthStencilKey) == 0)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
		depthStencilDesc.DepthEnable = currentDepthMode != Gpu::DepthMode_None;
		depthStencilDesc.DepthFunc = currentDepthMode == Gpu::DepthMode_Write ? D3D11_COMPARISON_ALWAYS : D3D11_COMPARISON_LESS_EQUAL;
		depthStencilDesc.DepthWriteMask = (currentDepthMode == Gpu::DepthMode_ReadWrite || currentDepthMode == Gpu::DepthMode_Write ?
			D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);

		ID3D11DepthStencilState * depthStencilState = 0;
		direct3Ddevice->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);

		depthStencilStates[depthStencilKey] = depthStencilState;
	}

	if(currentDepthStencilKey != depthStencilKey)
	{
		direct3Dcontext->OMSetDepthStencilState(depthStencilStates[depthStencilKey], 0);
		currentDepthStencilKey = depthStencilKey;
	}

	return true;
}

void DX11::Api::ResizeWindowResources(PlatformWindow * window, unsigned width, unsigned height)
{
	std::list<Gpu::IDeviceListener*>::iterator lostIt;
	for (lostIt = deviceListeners.begin(); lostIt != deviceListeners.end(); ++lostIt)
	{
		(*lostIt)->OnLostDevice(this);
	}

	WindowSurfaceMap::iterator it = windowDrawSurfaces.find(window);
	if (it != windowDrawSurfaces.end())
	{
		windowDrawSurfaces[window]->Release();
	}

	IDXGISwapChain * dxgiSwapChain = windowSwapChains[window];

	// For future reference, ResizeBuffers will accept w/h of 0/0 and size automatically...
	if (width > 0 && height > 0)
		dxgiSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ID3D11Texture2D* backBuffer;
	dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBuffer->GetDesc(&backBufferDesc);

	if (it != windowDrawSurfaces.end())
	{
		windowDrawSurfaces[window]->Resize(backBuffer, backBufferDesc.Width, backBufferDesc.Height);
	}
	else
	{
		windowDrawSurfaces[window] = new BackbufferSurface(direct3Ddevice, direct3Dcontext, backBuffer, backBufferDesc.Width, backBufferDesc.Height);
	}

	if (!mainDrawSurface) mainDrawSurface = windowDrawSurfaces[window];

	backBuffer->Release();

	std::list<Gpu::IDeviceListener*>::iterator resetIt;
	for (resetIt = deviceListeners.begin(); resetIt != deviceListeners.end(); ++resetIt)
	{
		(*resetIt)->OnResetDevice(this);
	}
}

void DX11::Api::Initialize(AssetMgr * assets)
{
	// Load the default shaders

	Files::Directory * frameworkDir = assets->GetFileApi()->GetKnownDirectory(Files::FrameworkDir);

	assets->Load(frameworkDir, L"BaseShader.xml", ShaderAsset, 0, AssetMgr::CRITICAL_TICKET);
	assets->Load(frameworkDir, L"TextureCopy.xml", ShaderAsset, 0, AssetMgr::CRITICAL_TICKET);
}

void DX11::Api::OnCriticalLoad(AssetMgr * assets)
{
	Files::Directory * frameworkDir = assets->GetFileApi()->GetKnownDirectory(Files::FrameworkDir);

	Gpu::Shader * gpuBaseShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"BaseShader.xml");
	Gpu::Shader * gpuTexCopyShader = assets->GetAsset<Gpu::Shader>(frameworkDir, L"TextureCopy.xml");

	baseEffect = new Gpu::Effect(gpuBaseShader);
	texCopyShader = static_cast<DX11::TextureShader*>(gpuTexCopyShader);
}

void DX11::Api::Clear() 
{
	mainDrawSurface->Clear(glm::vec4(clearColor[0],clearColor[1],clearColor[2],clearColor[3]));
}

void DX11::Api::BeginScene() 
{
	mainDrawSurface->Begin();

#ifdef USE_DIRECT2D
	direct2Dtarget->BeginDraw();
#endif

	SetBlendMode(Gpu::BlendMode_Alpha);

	currentDepthStencilKey = -1;
	SetDepthStencilState(mainDrawSurface);

	spriteBatch->Begin(SpriteSortMode_BackToFront,commonStates->NonPremultiplied());
}

void DX11::Api::EndScene()
{
	direct3Dcontext->GSSetShader(0, 0, 0);

	spriteBatch->End();

#ifdef USE_DIRECT2D
	direct2Dtarget->EndDraw();
#endif

	profileTimes.clear();

	currFrame = (currFrame + 1) % QUERY_LATENCY;

	float queryTime = 0.0f;

	// Iterate over all of the profiles
	ProfileMap::iterator iter;
	for(iter = profiles.begin(); iter != profiles.end(); iter++)
	{
		ProfileData& profile = (*iter).second;
		if(profile.queryFinished == FALSE)
			continue;

		profile.queryFinished = FALSE;

		if(profile.disjointQuery[currFrame] == NULL)
			continue;

		__int64 startTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTimeStamp);

		// Get the query data
		UINT64 startTime = 0;
		while(direct3Dcontext->GetData(profile.timestampStartQuery[currFrame], &startTime, sizeof(startTime), 0) != 0);

		UINT64 endTime = 0;
		while(direct3Dcontext->GetData(profile.timestampEndQuery[currFrame], &endTime, sizeof(endTime), 0) != 0);

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
		while(direct3Dcontext->GetData(profile.disjointQuery[currFrame], &disjointData, sizeof(disjointData), 0) != 0);

		__int64 curTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&curTimeStamp);

		queryTime += secsPerCnt * float(curTimeStamp - startTimeStamp);

		float time = 0.0f;
		if(disjointData.Disjoint == FALSE)
		{
			UINT64 delta = endTime - startTime;
			float frequency = static_cast<float>(disjointData.Frequency);
			time = (delta / frequency);
		}

		profile.drawCalls = 0;
		profile.stateChanges = 0;

		profileTimes[iter->first] = time;
	}

	//direct3Dcontext->RSSetState(0);

	// Reset all shader resource views to revert changes made by last SpriteBatch flush
	ID3D11ShaderResourceView * nullResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	memset(nullResources, 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT * sizeof(void*));
	direct3Dcontext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullResources);

	// Reset and force all sampler params to revert changes made by last SpriteBatch flush
	samplerMgr->ResetSamplerParams();
	samplerMgr->ApplySamplerParams(
		direct3Dcontext,
		Shader::ParamMap(),
		&baseEffect->samplerParams,
		true,
		true);

	// Release the backbuffers for any windows destroyed this frame
	for(unsigned i = 0; i < destroyedWindows.size(); ++i)
	{
		WindowSurfaceMap::iterator surfIt = windowDrawSurfaces.find(destroyedWindows[i]);
		WindowSwapChainMap::iterator swapIt = windowSwapChains.find(destroyedWindows[i]);

		delete surfIt->second;
		swapIt->second->Release();

		windowDrawSurfaces.erase(surfIt);
		windowSwapChains.erase(swapIt);
	}
	destroyedWindows.clear();

	// Resize the resources of any windows resized this frame
	for (unsigned i = 0; i < resizedWindows.size(); ++i)
	{
		PlatformWindow * window = resizedWindows[i];
		ResizeWindowResources(window, window->GetWidth(), window->GetHeight());
	}
	resizedWindows.clear();
}
void DX11::Api::Present()
{
	std::map<PlatformWindow*, IDXGISwapChain*>::iterator it;
	for(it = windowSwapChains.begin(); it != windowSwapChains.end(); ++it)
	{
		it->second->Present(0, 0);
	}
}

void DX11::Api::DrawGpuText(Gpu::Font* font, LPCWSTR text, float x, float y, bool center, Gpu::DrawSurface * surface) 
{
	if(!font) return;
	DX11::Font * dx11font = static_cast<DX11::Font*>(font);

	XMFLOAT2 position(x,y);
	XMFLOAT2 offset(0.0f,0.0f);
	XMVECTORF32 color = {font->color.r,font->color.g,font->color.b,font->color.a};
	float size = dx11font->fontSize/100.f;

	if(center)
	{
		XMFLOAT2 widthHeight;
		XMStoreFloat2(&widthHeight,dx11font->fontObject->MeasureString(text));
		// "MeasureString" seems to measure a string 20px thinner than it should be
		offset.x = (widthHeight.x + 20.0f)/2.0f; offset.y = widthHeight.y/2.0f;
	}

	float screenWidth = float(mainDrawSurface->width);
	float screenHeight = float(mainDrawSurface->height);

	if(!font->pixelSpace)
	{
		position.x = (screenWidth + ((2.0f * x) * screenHeight)) * 0.5f;
		position.y = (screenHeight + ((2.0f * y) * screenHeight)) * 0.5f;
		//size *= float(backBufferHeight)/float(standardScreenHeight);
	}

	DX11::DrawSurface * dx11surface = static_cast<DX11::DrawSurface*>(surface);
	if(dx11surface)
	{
		spriteBatch->End();
		dx11surface->Begin();
		spriteBatch->Begin(DirectX::SpriteSortMode_Immediate);
	}
	//spriteBatch->Begin();//SpriteSortMode_Deferred,0,0,depthStencilState);
	//if(!center)
	try
	{
		dx11font->fontObject->DrawString(spriteBatch, text, position, color * font->color.a,
			0.0f, offset, size);
	}
	catch(std::exception stringEx)
	{
		OutputDebugString(L"Failed to draw string!\n");
	}
	//spriteBatch->End();
	if(dx11surface)
	{
		spriteBatch->End();
		dx11surface->End();
		spriteBatch->Begin(SpriteSortMode_BackToFront, commonStates->NonPremultiplied());
	}

	//ID3D11BlendState blendState = 
	//direct3Dcontext->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
	//direct3Dcontext->OMSetDepthStencilState(0,0);
	//direct3Dcontext->RSSetState(0);

	ProfileList::iterator profileIt = activeProfiles.begin();
	for(; profileIt != activeProfiles.end(); profileIt++)
	{
		(*profileIt)->drawCalls++;
	}
}

void DX11::Api::DrawGpuModel(Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights, 
									 unsigned numLights, Gpu::DrawSurface * surface, 
									 Gpu::InstanceBuffer * instances, Gpu::Effect * overrideEffect) 
{
	// SANITY CHECKS - FREE //

	if(!model) return;
	if(!model->mesh) return;
	if(model->backFaceCull && model->frontFaceCull) return;
	if(!baseEffect) return;

	//if(numLights > 0) numLights = 1;

	DX11::Mesh * dx11mesh = static_cast<DX11::Mesh*>(model->mesh);

	// SETTING RASTERIZER STATE - MINOR COST - ~3us //

	if(model->wireframe || drawEverythingWireframe)
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
			rasterDesc.SlopeScaledDepthBias = 0.0f; // !!! TURNS OUT THIS IS CRUCIAL FOR SHADOW MAPPING !!!
			direct3Ddevice->CreateRasterizerState(&rasterDesc,&wireframeState);
		}
		direct3Dcontext->RSSetState(wireframeState);
	}
	else
	{
		direct3Dcontext->RSSetState(defaultRasterState);
	}
	
	// SETTING VERTEX & INDEX BUFFERS - MINOR COST - ~7us //

	direct3Dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // Not necessary - do this once ?

	UINT offset = 0;
	direct3Dcontext->IASetVertexBuffers(0, 1, &dx11mesh->vertexBuffer, &dx11mesh->vertexSize, &offset);

	if(dx11mesh->IsIndexed())
	{
		direct3Dcontext->IASetIndexBuffer(dx11mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}
	else
	{
		direct3Dcontext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	}

	// SETTING INSTANCE BUFFER - MINOR COST - ~7us //

	VertexType vertexType = dx11mesh->vertexType;
	InstanceType instanceType = InstanceType_None;

	if(instances)
	{
		DX11::InstanceBuffer * dx11instances = static_cast<DX11::InstanceBuffer*>(instances);
		direct3Dcontext->IASetVertexBuffers(1, 1, &dx11instances->buffer, &dx11instances->instanceSize, &offset);

		instanceType = dx11instances->GetType();
	}

	// GETTING EFFECT AND SHADER - FREE //

	Gpu::Effect * effect = baseEffect;

	if(overrideEffect && overrideEffect->shader && overrideEffect->shader->shaderType == Gpu::Shader::Type::Model)
	{
		effect = overrideEffect;
	}
	else if(model->effect && model->effect->shader && model->effect->shader->shaderType == Gpu::Shader::Type::Model) 
	{
		effect = model->effect;
	}

	DX11::ModelShader * dx11shader = static_cast<DX11::ModelShader*>(effect->shader);

	// GETTING TARGET AND CALCULATING ASPECT - UNKNOWN (FREE?) //

	DX11::DrawSurface * dx11surface = surface ? static_cast<DX11::DrawSurface*>(surface) : mainDrawSurface;
	
	float aspect = float(dx11surface->GetWidth()) / float(dx11surface->GetHeight());

	// SETTING DEPTH STENCIL STATE - UNKNOWN //

	SetDepthStencilState(dx11surface);

	// SWITCHING SHADER TECHNIQUE - MINOR COST - ~5us //

	if(!dx11shader->SetTechnique(direct3Dcontext, vertexType, instanceType)) return;

	// APPLYING SHADER PARAMS - SIGNIFICANT COST - ~32us //

	if(!dx11shader->SetParameters(direct3Dcontext, model, camera, lights, numLights, aspect, effect)) return;

	// APPLYING SAMPLER PARAMS - AMORTISED! //

	samplerMgr->ApplySamplerParams(
		direct3Dcontext, 
		dx11shader->currentTechnique->paramMappings,
		effect ? &effect->samplerParams : 0,
		true);

	// DRAWING - SIGNIFICANT COST - ~18us //

	if(dx11surface) dx11surface->Begin();
	if(dx11mesh->IsIndexed()) 
	{
		if(instances)
		{
			direct3Dcontext->DrawIndexedInstanced(dx11mesh->numTriangles * 3, instances->GetLength(), 0, 0, 0);
		}
		else
		{
			direct3Dcontext->DrawIndexed(dx11mesh->numTriangles * 3, 0, 0);
		}
	}
	else
	{
		if(instances)
		{
			direct3Dcontext->DrawInstanced(dx11mesh->numVertices, instances->GetLength(), 0, 0);
		}
		else
		{
			direct3Dcontext->Draw(dx11mesh->numVertices, 0);
		}
	}
	if(dx11surface)
	{
		dx11surface->End();
		mainDrawSurface->Begin();
	}

	dx11shader->UnsetParameters(direct3Dcontext, effect);

	if(instances)
	{
		direct3Dcontext->IASetVertexBuffers(1, 0, 0, 0, 0);
	}

	// UPDATING PROFILES - MINOR COST //

	ProfileList::iterator profileIt = activeProfiles.begin();
	for(; profileIt != activeProfiles.end(); profileIt++)
	{
		(*profileIt)->drawCalls++;
	}
}

void DX11::Api::DrawGpuSurface(Gpu::DrawSurface * source, Gpu::Effect * effect, Gpu::DrawSurface * dest)
{
	if(!source) return;
	if(!texCopyShader) return;
	
	UINT stride = texShaderQuad->vertexSize;
	UINT offset = 0;
	direct3Dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	direct3Dcontext->IASetVertexBuffers(0, 1, &texShaderQuad->vertexBuffer, &stride, &offset);
	direct3Dcontext->IASetIndexBuffer(texShaderQuad->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	direct3Dcontext->IASetInputLayout(texShaderLayout);
	direct3Dcontext->VSSetShader(texVertexShader,0,0);

	// TODO: Check this works with stencils
	SetDepthStencilState(dest ? static_cast<DX11::DrawSurface*>(dest) : mainDrawSurface);

	DX11::TextureShader * texShader = texCopyShader;
	if(effect && effect->shader)
	{
		if(!effect->shader->shaderType == Gpu::Shader::Type::Texture) return;
		texShader = static_cast<DX11::TextureShader*>(effect->shader);
	}
	texShader->SetParameters(direct3Dcontext, source->GetTexture(), effect);

	samplerMgr->ApplySamplerParams(
		direct3Dcontext, 
		texShader->paramMappings, 
		effect ? &effect->samplerParams : 0, 
		false);

	if(dest)
	{
		static_cast<DX11::TextureSurface*>(dest)->Begin();
	}

	direct3Dcontext->DrawIndexed(texShaderQuad->numTriangles * 3, 0, 0);

	if(dest)
	{
		static_cast<DX11::TextureSurface*>(dest)->End();
		mainDrawSurface->Begin();
	}

	ProfileList::iterator profileIt = activeProfiles.begin();
	for(; profileIt != activeProfiles.end(); profileIt++)
	{
		(*profileIt)->drawCalls++;
	}
}

void DX11::Api::DrawIndirect(Gpu::Effect * effect, Gpu::ParamBuffer * vertices, Gpu::ParamBuffer * instances, Gpu::DrawSurface * surface)
{
	if(!effect) return;

	DX11::ModelShader * dx11shader = static_cast<DX11::ModelShader*>(effect->shader);
	DX11::DrawSurface * dx11surface = surface ? static_cast<DX11::DrawSurface*>(surface) : mainDrawSurface;
	float aspect = float(dx11surface->GetWidth()) / float(dx11surface->GetHeight());

	direct3Dcontext->IASetVertexBuffers(0, 0, 0, 0, 0);
	direct3Dcontext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	SetDepthStencilState(dx11surface);

	if(!dx11shader->SetIndirectTechnique(direct3Dcontext)) return;
	if(!dx11shader->indirectTechnique.SetExtraParameters(direct3Dcontext, effect)) return;

	samplerMgr->ApplySamplerParams(
		direct3Dcontext,
		dx11shader->indirectTechnique.paramMappings,
		effect ? &effect->samplerParams : 0,
		true);

	if(vertices || instances)
	{
		ID3D11Buffer * indirectBuffer = dx11shader->GetIndirectBuffer();

		if(vertices)
		{
			DX11::ParamBuffer * dx11vertices = static_cast<DX11::ParamBuffer*>(vertices);

			direct3Dcontext->CopyStructureCount(indirectBuffer, 0, dx11vertices->uav);
		}
		if(instances)
		{
			DX11::ParamBuffer * dx11instances = static_cast<DX11::ParamBuffer*>(instances);

			direct3Dcontext->CopyStructureCount(indirectBuffer, sizeof(UINT), dx11instances->uav);
		}

		direct3Dcontext->DrawInstancedIndirect(indirectBuffer, 0);
	}
	else
	{
		OutputDebugString(L"DrawAuto: Not tested yet!");
		if(IsDebuggerPresent()) __debugbreak();

		direct3Dcontext->DrawAuto();
	}

	dx11shader->UnsetParameters(direct3Dcontext, effect);
}

void DX11::Api::Compute(Gpu::Effect * effect, unsigned groupX, unsigned groupY, unsigned groupZ)
{
	if(!effect || !effect->shader) return;
	if(groupX < 1 || groupY < 1 || groupZ < 1) return;

	DX11::ComputeShader * computeShader = static_cast<DX11::ComputeShader*>(effect->shader);

	computeShader->SetParameters(direct3Dcontext, effect);

	direct3Dcontext->Dispatch(groupX, groupY, groupZ);

	computeShader->UnsetParameters(direct3Dcontext, effect);

	// Bear in mind, OpenCL clEnqueueNDRangeKernel works slightly differently.

	// Where DirectCompute takes exact values for the NUMBER OF GROUPS and MULTIPLIES
	// that by the number of THREADS PER GROUP to get the total number of threads...

	// OpenCL takes values for the total NUMBER OF THREADS and DIVIDES that by the 
	// number of THREADS PER GROUP to get the number of groups!

	// Therefore, as specified in the documentation here:
	// https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clEnqueueNDRangeKernel.html
	// And the StackOverflow here:
	// http://stackoverflow.com/questions/25237145/opencl-clenqueuendrangekernel-how-to-set-work-group-size-correctly
	// The total NUMBER OF THREADS must be evenly divisible by the number of THREADS PER GROUP!

	// Looks like the best way to do this is to specify the number of threads per group
	// for OpenCL in the shader metadata, and multiply that by the number of groups to feed
	// the total number of threads into clEnqueueNDRangeKernel. 
	// We can't pass the number of threads per group to the function, because in DirectCompute,
	// these values are hard-coded into the shader itself! Adding the OpenCL threads per group
	// to the shader metadata keeps the interface and workflow consistent and simple.
}

Gpu::Font * DX11::Api::CreateGpuFont(int height, LPCWSTR facename, Gpu::FontStyle style) 
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
	case Gpu::FontStyle_Bold:
		ss << L"B";
		break;
	case Gpu::FontStyle_Italic:
		ss << L"I";
		break;
	default:
		break;
	}
	ss << L".spritefont";
	std::wstring fileString = ss.str();

	// TODO: Add relative directory??? Multiple path search???
	try
	{
		SpriteFont * font = new SpriteFont(direct3Ddevice, fileString.c_str());
		return new DX11::Font(font, facename, ((float)height));//((float)height)/(95.f));
	}
	catch(std::exception ex)
	{
		return 0;
	}
#endif
}

Gpu::Texture * DX11::Api::CreateGpuTexture(char * data, unsigned dataSize, bool isDDS)
{
	ID3D11ShaderResourceView * shaderView = 0;
	ID3D11Resource * resource = 0;

	if(isDDS)
	{
		CreateDDSTextureFromMemory(direct3Ddevice, (const uint8_t*)data, dataSize, &resource, &shaderView);
	}
	else
	{
		CreateWICTextureFromMemory(direct3Ddevice, direct3Dcontext, (const uint8_t*) data, dataSize, &resource, &shaderView);
	}

	if(resource)
	{
		ID3D11Texture2D * texture2D = 0;
		resource->QueryInterface(__uuidof(ID3D11Texture2D),(void**)&texture2D);
		resource->Release();
		if(texture2D)
		{
			D3D11_TEXTURE2D_DESC desc;
			texture2D->GetDesc(&desc);
			if((desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == 0)
			{
				return new DX11::Texture(texture2D, shaderView, desc);
			}
			texture2D->Release();
		}
	}
	if(shaderView) shaderView->Release();
	return 0;
}

Gpu::Texture * DX11::Api::CreateGpuTexture(char * data, unsigned dataSize, unsigned width, unsigned height)
{
	ID3D11ShaderResourceView * shaderView = 0;
	ID3D11Texture2D * texture = nullptr;

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = data;
	initData.SysMemPitch = static_cast<UINT>((width * 32 + 7) / 8);
	initData.SysMemSlicePitch = static_cast<UINT>(dataSize);

	HRESULT hr = direct3Ddevice->CreateTexture2D(&desc, &initData, &texture);
	if(SUCCEEDED(hr) && texture != 0)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		memset(&SRVDesc, 0, sizeof(SRVDesc));
		SRVDesc.Format = desc.Format;

		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = direct3Ddevice->CreateShaderResourceView(texture, &SRVDesc, &shaderView);
		if(FAILED(hr))
		{
			texture->Release();
			return 0;
		}

		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		return new DX11::Texture(texture, shaderView, desc);
	}

	return 0;
}

Gpu::CubeMap * DX11::Api::CreateGpuCubeMap(char * data, unsigned dataSize)
{
	ID3D11Resource * resource = 0;
	ID3D11ShaderResourceView * shaderView = 0;

	CreateDDSTextureFromMemory(direct3Ddevice, (const uint8_t*) data, dataSize, &resource, &shaderView);

	if(resource)
	{
		ID3D11Texture2D * texture2D;
		resource->QueryInterface(__uuidof(ID3D11Texture2D),(void**)&texture2D);
		resource->Release();
		if(texture2D)
		{
			D3D11_TEXTURE2D_DESC desc;
			texture2D->GetDesc(&desc);
			if(desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
			{
				return new DX11::CubeMap(texture2D, shaderView, desc);
			}
			texture2D->Release();
		}
	}
	if(shaderView) shaderView->Release();
	return 0;
}

Gpu::VolumeTexture * DX11::Api::CreateGpuVolumeTexture(char * data, unsigned dataSize)
{
	ID3D11ShaderResourceView * shaderView = 0;
	ID3D11Resource * resource = 0;

	CreateDDSTextureFromMemory(direct3Ddevice, (const uint8_t*) data, dataSize, &resource, &shaderView);

	if(resource)
	{
		ID3D11Texture3D * texture3D;
		resource->QueryInterface(__uuidof(ID3D11Texture3D),(void**)&texture3D);
		resource->Release();
		if(texture3D)
		{
			D3D11_TEXTURE3D_DESC desc;
			texture3D->GetDesc(&desc);
			return new DX11::VolumeTexture(texture3D, shaderView, desc);
		}
	}
	if(shaderView) shaderView->Release();
	return 0;
}

Gpu::ShaderLoader * DX11::Api::CreateGpuShaderLoader(Files::Api * files, Files::Directory * directory, const wchar_t * path)
{
	return new DX11::ShaderLoader(this, files, directory, path);
}

Gpu::Mesh * DX11::Api::CreateGpuMesh(unsigned numVertices, void * vertexData, VertexType type, bool dynamic) 
{
	DX11::Mesh * createdMesh = new DX11::Mesh();

	D3D11_BUFFER_DESC vbdesc;
	vbdesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	vbdesc.ByteWidth = VertApi::GetVertexSize(type) * numVertices;
	vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbdesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	vbdesc.MiscFlags = 0;
	vbdesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vbInitData;
	vbInitData.pSysMem = vertexData;
	direct3Ddevice->CreateBuffer(&vbdesc,&vbInitData,&createdMesh->vertexBuffer);

	createdMesh->vertexSize = VertApi::GetVertexSize(type);
	createdMesh->numVertices = numVertices;
	createdMesh->vertexType = type;
	createdMesh->dynamic = dynamic;

	return createdMesh;
}

Gpu::Mesh * DX11::Api::CreateGpuMesh(unsigned numVertices, void * vertexData,
	unsigned numTriangles, unsigned * indexData, VertexType type, bool dynamic) 
{
	DX11::Mesh * createdMesh = static_cast<DX11::Mesh*>(CreateGpuMesh(numVertices,vertexData,type,dynamic));

	D3D11_BUFFER_DESC ibdesc;
	ibdesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	ibdesc.ByteWidth = sizeof(unsigned) * numTriangles * 3;
	ibdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibdesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	ibdesc.MiscFlags = 0;
	ibdesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA ibInitData = {0};
	ibInitData.pSysMem = indexData;
	direct3Ddevice->CreateBuffer(&ibdesc,&ibInitData,&createdMesh->indexBuffer);

	createdMesh->numTriangles = numTriangles;

	return createdMesh;
}

//Gpu::Mesh * DX11::Api::CreateGpuMesh(GenericVertexBuffer * vertexBuffer)
//{
//	// Need to pass in a collection of buffers to merge into an input description
//
//	static const char * componentSemanticToGpuSemantic[GenericVertexBuffer::SemanticCount] =
//	{
//		"POSITION",
//		"COLOR",
//		"TEXTURE",
//		"NORMAL",
//		"TEXTURE",
//		"TEXTURE"
//	};
//
//	static const DXGI_FORMAT floatCountToDxgiFormat[5] =
//	{
//		DXGI_FORMAT_UNKNOWN,
//		DXGI_FORMAT_R32_FLOAT,
//		DXGI_FORMAT_R32G32_FLOAT,
//		DXGI_FORMAT_R32G32B32_FLOAT,
//		DXGI_FORMAT_R32G32B32A32_FLOAT
//	};
//
//	const std::vector<GenericVertexBuffer::ComponentMeta> & componentMeta = vertexBuffer->GetComponentMeta();
//	D3D11_INPUT_ELEMENT_DESC * inputElementDesc = new D3D11_INPUT_ELEMENT_DESC[componentMeta.size()];
//
//	for(unsigned i = 0; i < componentMeta.size(); ++i)
//	{
//		// We need to be able to build this on the fly!
//
//		inputElementDesc[i] = { componentSemanticToGpuSemantic[componentMeta[i].semantic], 
//			0, floatCountToDxgiFormat[componentMeta[i].size], 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
//	}
//}

Gpu::InstanceBuffer * DX11::Api::CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type)
{
	DX11::InstanceBuffer * instanceBuffer = new DX11::InstanceBuffer(direct3Ddevice, type, numInstances, instanceData);

	if(instanceBuffer->buffer)
	{
		return instanceBuffer;
	}

	delete instanceBuffer;
	return 0;
}

// To identify the buffer as an append/consume buffer, initialCount must be > -1
Gpu::ParamBuffer * DX11::Api::CreateParamBuffer(unsigned numElements, void * data, unsigned structureStride, unsigned initialCount)
{
	ID3D11Buffer * buffer = 0;
	ID3D11ShaderResourceView * srv = 0;
	ID3D11UnorderedAccessView * uav = 0;

	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.ByteWidth = structureStride > 0 ? numElements * structureStride : numElements;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.MiscFlags = structureStride > 0 ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	bufferDesc.StructureByteStride = structureStride;

	D3D11_SUBRESOURCE_DATA bufferData = { 0 };
	bufferData.pSysMem = data;

	direct3Ddevice->CreateBuffer(&bufferDesc, data ? &bufferData : 0, &buffer);

	if(buffer)
	{
		// Create a UAV and SRV to allow use both as a source of 
		// data and a destination for updated data.

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		if(structureStride > 0)
		{
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.Buffer.NumElements = numElements;
			if(initialCount < UINT_MAX)
			{
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
			}
			else
			{
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
			}
		}
		else
		{
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			uavDesc.Buffer.NumElements = numElements / 4;
		}

		direct3Ddevice->CreateUnorderedAccessView(buffer, &uavDesc, &uav);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = 0;
		if(structureStride > 0)
		{
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.BufferEx.NumElements = numElements;
		}
		else
		{
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			srvDesc.BufferEx.NumElements = numElements / 4;
		}

		direct3Ddevice->CreateShaderResourceView(buffer, &srvDesc, &srv);

		ParamBuffer * result = new ParamBuffer(buffer, srv, uav);

		direct3Dcontext->CSSetUnorderedAccessViews(0, 1, &uav, &(initialCount < UINT_MAX ? initialCount : numElements));

		ID3D11UnorderedAccessView * nulluav = 0;
		unsigned zero = 0;
		direct3Dcontext->CSSetUnorderedAccessViews(0, 1, &nulluav, &zero);

		return result;
	}

	return 0;
}

void DX11::Api::CopyParamBufferSize(Gpu::ParamBuffer * src, Gpu::ParamBuffer * dst, unsigned byteOffset)
{
	DX11::ParamBuffer * dx11srcBuf = static_cast<DX11::ParamBuffer*>(src);
	DX11::ParamBuffer * dx11dstBuf = static_cast<DX11::ParamBuffer*>(dst);

	direct3Dcontext->CopyStructureCount(dx11dstBuf->buffer, byteOffset, dx11srcBuf->uav);
}

void DX11::Api::GetParamBufferData(Gpu::ParamBuffer * src, void * dst, unsigned offset, unsigned numBytes)
{
	DX11::ParamBuffer * dx11buf = static_cast<DX11::ParamBuffer*>(src);

	ID3D11Buffer * stagingBuf = 0;

	direct3Ddevice->CreateBuffer(&CD3D11_BUFFER_DESC(numBytes, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ), 0, &stagingBuf);

	direct3Dcontext->CopySubresourceRegion(stagingBuf, 0, 0, 0, 0, dx11buf->buffer, 0, &CD3D11_BOX(offset, 0, 0, offset + numBytes, 1, 1));

	D3D11_MAPPED_SUBRESOURCE mappedData = { 0 };
	direct3Dcontext->Map(stagingBuf, 0, D3D11_MAP_READ, 0, &mappedData);
	memcpy(dst, mappedData.pData, numBytes);
	direct3Dcontext->Unmap(stagingBuf, 0);

	stagingBuf->Release();
}

void DX11::Api::UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, IVertexBuffer * buffer)
{
	DX11::Mesh * dx11mesh = static_cast<DX11::Mesh*>(dynamicMesh);

	if(!dx11mesh->dynamic)
	{
		OutputDebugString(L"Attempted to update a non-dynamic mesh\n");
		return;
	}
	
	if(buffer->GetLength() > dx11mesh->numVertices)
	{
		OutputDebugString(L"Attempted to overflow a dynamic vertex buffer\n");
		return;
	}

	if(buffer->GetVertexType() != dx11mesh->vertexType)
	{
		OutputDebugString(L"Attempted to update a dynamic vertex buffer of a different type");
		return;
	}
	
	D3D11_MAPPED_SUBRESOURCE mappedData = {0};
	direct3Dcontext->Map(dx11mesh->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy(mappedData.pData, buffer->GetData(), buffer->GetLength() * buffer->GetElementSize());
	direct3Dcontext->Unmap(dx11mesh->vertexBuffer, 0);
}

void DX11::Api::UpdateDynamicMesh(Gpu::Mesh * dynamicMesh, unsigned numTriangles, unsigned * indexData)
{
	DX11::Mesh * dx11mesh = static_cast<DX11::Mesh*>(dynamicMesh);

	if(numTriangles > dx11mesh->numTriangles)
	{
		OutputDebugString(L"Attempted to overflow a dynamic index buffer\n");
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedData = {0};
	direct3Dcontext->Map(dx11mesh->indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy(mappedData.pData, indexData, numTriangles * sizeof(unsigned) * 3);
	direct3Dcontext->Unmap(dx11mesh->indexBuffer, 0);
}

void DX11::Api::UpdateInstanceBuffer(Gpu::InstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData)
{
	DX11::InstanceBuffer * dx11instanceBuffer = static_cast<DX11::InstanceBuffer*>(instanceBuffer);

	if(numInstances > dx11instanceBuffer->instanceCapacity)
	{
		OutputDebugString(L"Attempted to overflow a smaller instance buffer\n");
		return;
	}

	dx11instanceBuffer->numInstances = numInstances;

	unsigned instanceDataSize = VertApi::GetInstanceSize(dx11instanceBuffer->type) * numInstances;
	
	D3D11_MAPPED_SUBRESOURCE mappedData = {0};
	direct3Dcontext->Map(dx11instanceBuffer->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy(mappedData.pData, instanceData, instanceDataSize);
	direct3Dcontext->Unmap(dx11instanceBuffer->buffer, 0);
}

Gpu::DrawSurface * DX11::Api::CreateDrawSurface(unsigned width, unsigned height, Gpu::DrawSurface::Format format)
{
	return new DX11::TextureSurface(this, direct3Ddevice, direct3Dcontext, format, false, float(width), float(height));
}

Gpu::DrawSurface * DX11::Api::CreateRelativeDrawSurface(PlatformWindow * window, float widthFactor, float heightFactor, Gpu::DrawSurface::Format format)
{
	DX11::TextureSurface * surface = new DX11::TextureSurface(this, direct3Ddevice, direct3Dcontext, format, window, widthFactor, heightFactor);
	deviceListeners.push_back(surface);
	return surface;
}

Gpu::DrawSurface * DX11::Api::GetWindowDrawSurface(PlatformWindow * window)
{
	if(!window) return mainDrawSurface;

	WindowSurfaceMap::iterator it = windowDrawSurfaces.find(window);
	if(it != windowDrawSurfaces.end())
	{
		return it->second;
	}
	return 0;
}

void DX11::Api::BeginTimestamp(const std::wstring name)
{
	ProfileData& profileData = profiles[name];
	if(profileData.queryStarted || profileData.queryFinished) return;

	if(profileData.disjointQuery[currFrame] == NULL)
	{
		// Create the queries
		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		desc.MiscFlags = 0;
		direct3Ddevice->CreateQuery(&desc, &profileData.disjointQuery[currFrame]);

		desc.Query = D3D11_QUERY_TIMESTAMP;
		direct3Ddevice->CreateQuery(&desc, &profileData.timestampStartQuery[currFrame]);
		direct3Ddevice->CreateQuery(&desc, &profileData.timestampEndQuery[currFrame]);
	}

	// Start a disjoint query first
	direct3Dcontext->Begin(profileData.disjointQuery[currFrame]);

	// Insert the start timestamp    
	direct3Dcontext->End(profileData.timestampStartQuery[currFrame]);

	profileData.queryStarted = TRUE;

	activeProfiles.push_front(&profileData);
}

void DX11::Api::EndTimestamp(const std::wstring name)
{
	ProfileData& profileData = profiles[name];
	if(!profileData.queryStarted || profileData.queryFinished) return;

	// Insert the end timestamp    
	direct3Dcontext->End(profileData.timestampEndQuery[currFrame]);

	// End the disjoint query
	direct3Dcontext->End(profileData.disjointQuery[currFrame]);

	profileData.queryStarted = FALSE;
	profileData.queryFinished = TRUE;

	activeProfiles.remove(&profileData);
}

Gpu::TimestampData DX11::Api::GetTimestampData(const std::wstring name)
{
	ProfileData& profileData = profiles[name];
	if(profileData.queryStarted) return Gpu::TimestampData();
	//_ASSERT(profileData.queryStarted == FALSE);
	//_ASSERT(profileData.queryFinished == TRUE);

	Gpu::TimestampData timestampData;
	timestampData.data[Gpu::TimestampData::Time] = profileTimes.find(name) != profileTimes.end() ? profileTimes[name] : 0.0f;
	timestampData.data[Gpu::TimestampData::DrawCalls] = float(profileData.drawCalls);
	timestampData.data[Gpu::TimestampData::StateChanges] = float(profileData.stateChanges);

	return timestampData;
}

void DX11::Api::SetClearColor(float r, float g, float b, float a)
{
	clearColor[0] = r; clearColor[1] = g; clearColor[2] = b; clearColor[3] = a;
}

void DX11::Api::OnWindowCreated(PlatformWindow * window)
{
#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP

	struct OnDestroyResponse : Win32::WindowEventResponse
	{
		Gpu::Api * gpu;
		OnDestroyResponse(Gpu::Api * gpu) : gpu(gpu) {}
		void Respond(Win32::Window * window, WPARAM wparam, LPARAM lparam) override
		{
			gpu->OnWindowDestroyed(window);
		}
	};

	struct OnResizeResponse : Win32::WindowEventResponse
	{
		Gpu::Api * gpu;
		OnResizeResponse(Gpu::Api * gpu) : gpu(gpu) {}
		void Respond(Win32::Window * window, WPARAM wparam, LPARAM lparam) override
		{
			gpu->OnWindowResized(window, window->GetWidth(), window->GetHeight());
		}
	};

	struct OnDeleteResponse : Win32::WindowEventResponse
	{
		void Respond(Win32::Window * window, WPARAM wparam, LPARAM lparam) override
		{
			delete window->onDestroy;
			delete window->onExitSizeMove;
			delete window->onDelete;
		}
	};

	Win32::Window * win32Window = static_cast<Win32::Window*>(window);

	win32Window->onDestroy = new OnDestroyResponse(this);
	win32Window->onExitSizeMove = new OnResizeResponse(this);
	win32Window->onResizing = win32Window->onExitSizeMove;
	win32Window->onMaximize = win32Window->onExitSizeMove;
	win32Window->onUnmaximize = win32Window->onExitSizeMove;
	win32Window->onDelete = new OnDeleteResponse();

	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	//swapChainDesc.Width = 0;                                     // use automatic sizing
	//swapChainDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // this is the most common swapchain format
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	//swapChainDesc.Stereo = false; 
	swapChainDesc.SampleDesc.Count = 1;                            // don't use multi-sampling
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.Windowed = true;
	swapChainDesc.OutputWindow = win32Window->GetHandle();

	IDXGIDevice* dxgiDevice = 0;
	direct3Ddevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	IDXGIAdapter* dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	IDXGIFactory* dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	IDXGISwapChain * dxgiSwapChain = 0;

	dxgiFactory->CreateSwapChain(
		dxgiDevice,
		&swapChainDesc,
		&dxgiSwapChain);

	windowSwapChains[window] = dxgiSwapChain;

	if(!mainSwapChain) mainSwapChain = dxgiSwapChain;
#else
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
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
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	IDXGIFactory2* dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);

	dxgiFactory->CreateSwapChainForCoreWindow(
		dxgiDevice,
		reinterpret_cast<IUnknown*>(window),
		&swapChainDesc,
		nullptr,
		&mainSwapChain);
#endif

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();
}

void DX11::Api::OnWindowResized(PlatformWindow * window, unsigned width, unsigned height) 
{
	resizedWindows.push_back(window);
}

void DX11::Api::OnWindowDestroyed(PlatformWindow * window)
{
	destroyedWindows.push_back(window);

	std::vector<PlatformWindow*>::iterator it = resizedWindows.begin();
	for (; it != resizedWindows.end(); ++it)
	{
		if ((*it) == window)
		{
			it = resizedWindows.erase(it);
		}
	}
}

void DX11::Api::GetBackbufferSize(unsigned & width, unsigned & height, PlatformWindow * window)
{
	WindowSurfaceMap::iterator it = windowDrawSurfaces.find(window);
	if(it != windowDrawSurfaces.end())
	{
		width = it->second->width; height = it->second->height;
	}
	else
	{
		width = mainDrawSurface->width; height = mainDrawSurface->height;
	}
}

void DX11::Api::SetMultisampling(unsigned multisampleLevel)
{
	// Need to rebuild the swap chain!!

	OutputDebugString(L"Not yet implemented");
	//if(IsDebuggerPresent()) __debugbreak();
}

void DX11::Api::SetBlendMode(Gpu::BlendMode blendMode)
{
	ID3D11BlendState * blendState = 0;

	if(blendStates[blendMode] == 0)
	{
		D3D11_BLEND_DESC desc = { 0 };

		D3D11_BLEND srcBlend;
		D3D11_BLEND dstBlend;

		switch(blendMode)
		{
		case Gpu::BlendMode_None:
			srcBlend = D3D11_BLEND_ONE;
			dstBlend = D3D11_BLEND_ZERO;
			break;
		case Gpu::BlendMode_Additive:
			srcBlend = D3D11_BLEND_SRC_ALPHA;
			dstBlend = D3D11_BLEND_ONE;
			break;
		case Gpu::BlendMode_Premultiplied:
			srcBlend = D3D11_BLEND_ONE;
			dstBlend = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		default:
			srcBlend = D3D11_BLEND_SRC_ALPHA;
			dstBlend = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		}

		desc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) || (dstBlend != D3D11_BLEND_ZERO);

		desc.RenderTarget[0].SrcBlend = srcBlend;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = dstBlend;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		direct3Ddevice->CreateBlendState(&desc, &blendState);
		blendStates[blendMode] = blendState;
	}
	else
	{
		blendState = blendStates[blendMode];
	}

	direct3Dcontext->OMSetBlendState(blendState, 0, 0xFFFFFFFF);
}

void DX11::Api::SetDepthMode(Gpu::DepthMode depthMode)
{
	currentDepthMode = depthMode;
}

bool DX11::Api::isDeviceLost() 
{
	return false;
}

void DX11::Api::ResetRenderTargets()
{
	mainDrawSurface->Begin();
}

float DX11::Api::MeasureGpuText(Gpu::Font* font, const wchar_t* text)
{
	DX11::Font * dx11font = static_cast<DX11::Font*>(font);
	XMFLOAT2 dimensions; 
	XMStoreFloat2(&dimensions, dx11font->fontObject->MeasureString(text));

	//if(font->pixelSpace)
		return dimensions.x * (dx11font->fontSize/95.f);
	//else
	//	return dimensions.x * 2.0f * (dx11font->fontSize/95.f)/((float)standardScreenHeight);
}

DX11::Font::~Font()
{
	if(fontObject) delete fontObject;
}

} // namespace Ingenuity

#endif // USE_DX11_GPUAPI
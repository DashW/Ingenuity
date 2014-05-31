#include "stdafx.h"

#ifdef USE_DX9_GPUAPI

#include "DX9Api.h"
#include "DX9VertApi.h"
#include <windef.h>
#include <sstream>
#include <vector>

D3DDEVTYPE mDevType;
DX9_GpuApi::DX9_GpuApi(HWND windowHandle) : 
	GpuApi(),
	direct3D(0),
	direct3Ddevice(0),
	spriteInterface(0),
	baseShader(0),
	view(),
	projection(),
	presentParameters(),
	deviceDependentFonts(),
	clearColor(0xffffffff)
{
	direct3D = Direct3DCreate9(D3D_SDK_VERSION);
	if( !direct3D )
	{
		initialised = false;
		return;
	}

	mDevType = D3DDEVTYPE_HAL;

	//D3DDISPLAYMODE mode;
	//direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&mode);

	// Check device capabilities

	D3DDISPLAYMODE mode;
	direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	direct3D->CheckDeviceType(D3DADAPTER_DEFAULT, mDevType, mode.Format, mode.Format, true);
	direct3D->CheckDeviceType(D3DADAPTER_DEFAULT, mDevType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false);
	
	D3DCAPS9 caps;
	direct3D->GetDeviceCaps(D3DADAPTER_DEFAULT, mDevType, &caps);

	DWORD devBehaviorFlags = 0;
	if( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
		devBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		devBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// If pure device and HW T&L supported
	if( caps.DevCaps & D3DDEVCAPS_PUREDEVICE &&
		devBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
			devBehaviorFlags |= D3DCREATE_PUREDEVICE;

	// Build Present Parameters

	presentParameters.BackBufferWidth = 0;
	presentParameters.BackBufferHeight = 0;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.BackBufferCount = 1;
	presentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	presentParameters.MultiSampleQuality = 0;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.hDeviceWindow = windowHandle;
	presentParameters.Windowed = true;
	presentParameters.EnableAutoDepthStencil = true;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D24S8;
	presentParameters.Flags = 0;
	presentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Create Device

	direct3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		mDevType,
		windowHandle, 
		devBehaviorFlags, 
		&presentParameters,
		&direct3Ddevice);

	DX9_VertApi::DeclareVertices(direct3Ddevice);

	D3DXCreateSprite(direct3Ddevice,&spriteInterface);

	// Create Shader

	ID3DXBuffer* errors = 0;
	D3DXCreateEffectFromFile(direct3Ddevice, COMPILED_SHADER_PATH, 0, 0, D3DXSHADER_DEBUG,
		0, &baseShader, &errors);
	if( errors ) MessageBox(0, (LPCWSTR)errors->GetBufferPointer(), 0, 0);

	initialised = true;
}
DX9_GpuApi::~DX9_GpuApi()
{
	if(baseShader) baseShader->Release(); baseShader = 0;
	if(spriteInterface) spriteInterface->Release();
	DX9_VertApi::ReleaseVertices();
	if(direct3Ddevice) direct3Ddevice->Release(); direct3Ddevice = 0;
	if(direct3D) direct3D->Release(); direct3D = 0;
}

void DX9_GpuApi::SetShaderFloat(const char* name, float value)
{
	D3DXHANDLE param = baseShader->GetParameterByName(0, name);
	baseShader->SetFloat(param, value);
}
void DX9_GpuApi::SetShaderVec3(const char* name, float x, float y, float z)
{
	D3DXHANDLE param = baseShader->GetParameterByName(0, name);
	D3DXVECTOR3 vec3(x,y,z);
	baseShader->SetValue(param, &vec3, sizeof(D3DXVECTOR3));
}
void DX9_GpuApi::SetShaderColor(const char* name, float r, float g, float b, float a)
{
	D3DXHANDLE param = baseShader->GetParameterByName(0, name);
	D3DXCOLOR color(r,g,b,a);
	baseShader->SetValue(param,&color,sizeof(D3DXCOLOR));
}

void DX9_GpuApi::LoadShaders(FileMgr* files)
{

}

void DX9_GpuApi::Clear()
{
	direct3Ddevice->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, clearColor, 1.0f, 0);
}
void DX9_GpuApi::BeginScene()
{
	direct3Ddevice->BeginScene();
}
void DX9_GpuApi::EndScene()
{
	direct3Ddevice->EndScene();
}
void DX9_GpuApi::Present()
{
	direct3Ddevice->Present(0,0,0,0);
}

void DX9_GpuApi::DrawGpuText(GpuFont* font, LPCWSTR text, float x, float y, bool center)
{
	// DEVICE-INDEPENDENT TEXT RENDERING NOT SUPPORTED IN DIRECTX 9
	if(!font->pixelSpace) return;

	DX9_GpuFont* dxfont = static_cast<DX9_GpuFont*>(font);
	RECT rect = {0};
	dxfont->fontObject->DrawTextW(0, text, -1, &rect, DT_CALCRECT, 
		floatToD3DColor(1.0,1.0,1.0,1.0));
	
	rect.left = (long) x; rect.top = long(y);

	if(center)
	{
		rect.left -= rect.right/2; rect.top -= rect.bottom/2;
	}

	rect.right += rect.left; rect.bottom += rect.top;

	dxfont->fontObject->DrawTextW(0, text, -1, &rect, DT_LEFT | DT_TOP, 
		floatToD3DColor(dxfont->colorR,dxfont->colorG,dxfont->colorB,dxfont->colorA));
}
void DX9_GpuApi::DrawGpuSprite(GpuSprite* sprite)
{
	if(!spriteInterface) return;

	//// The following code specifies an alpha test and reference value.
	direct3Ddevice->SetRenderState(D3DRS_ALPHAREF, 10);
	direct3Ddevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	if(sprite->brightAsAlpha)
	{
		direct3Ddevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_ADD);
		direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		direct3Ddevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		direct3Ddevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	}
	else
	{
		direct3Ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		direct3Ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
		direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		direct3Ddevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		direct3Ddevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
	}

	direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	direct3Ddevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	DX9_GpuTexture* texture = static_cast<DX9_GpuTexture*>(sprite->texture);
	D3DXVECTOR3 center(sprite->transformCenterX,sprite->transformCenterY,0.0f);
	D3DXVECTOR3 position(0.0f,0.0f,0.0f);
	position.z = sprite->positionZ;
	float w = static_cast<float>(presentParameters.BackBufferWidth/2);
	float h = static_cast<float>(presentParameters.BackBufferHeight/2);
	if(sprite->pixelSpace)
	{
		position.x = sprite->positionX; 
		position.y = sprite->positionY; 
	}
	else
	{
		position.x = w + (sprite->positionX * h);
		position.y = h + (sprite->positionY * h);
	}

	D3DXMATRIX centerMatrix, translationMatrix, rotationMatrix, scaleMatrix, screenMatrix, transformMatrix;

	D3DXMatrixTranslation(&centerMatrix,-center.x,-center.y,0.0f);
	D3DXMatrixTranslation(&translationMatrix, position.x, position.y, position.z);
	D3DXMatrixRotationZ(&rotationMatrix, sprite->rotation);
	D3DXMatrixScaling(&scaleMatrix, sprite->size, sprite->size, 1.0f);
	if(!sprite->pixelSpace)
	{
		float factor = (((float)presentParameters.BackBufferHeight)/((float)standardScreenHeight));
		D3DXMatrixScaling(&screenMatrix, factor, factor, 1.0f);
	}
	else
	{
		D3DXMatrixIdentity(&screenMatrix);
	}

	transformMatrix = (centerMatrix * rotationMatrix * scaleMatrix * screenMatrix) * translationMatrix;

	spriteInterface->SetTransform(&transformMatrix);

	spriteInterface->Begin(0);
	spriteInterface->Draw(texture->textureObject,0,0,0,0xffffffff);
	spriteInterface->Flush();
	spriteInterface->End();

	direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	direct3Ddevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
}
void DX9_GpuApi::DrawGpuMesh(GpuMesh* mesh, GpuCamera* camera, GpuLight** lights, 
						  unsigned numLights, bool wireFrame, GpuDrawBuffer* buffer)
{
}
void DX9_GpuApi::DrawGpuIndexedMesh(GpuIndexedMesh* mesh, GpuCamera* camera, GpuLight** lights, 
								 unsigned numLights, bool wireFrame, GpuDrawBuffer* buffer)
{
	// direct3Ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	DX9_GpuIndexedMesh* dx9mesh = static_cast<DX9_GpuIndexedMesh*>(mesh);
	direct3Ddevice->SetStreamSource(0, dx9mesh->vertexBuffer, 0, dx9mesh->vertexSize);
	direct3Ddevice->SetIndices(dx9mesh->indexBuffer);
	direct3Ddevice->SetVertexDeclaration(dx9mesh->vertexDeclaration);

	D3DXHANDLE technique;
	switch(dx9mesh->vertexType)
	{
	case VertexType_PosCol:
		technique = baseShader->GetTechniqueByName("posCol");
		break;
	case VertexType_PosNor:
		technique = baseShader->GetTechniqueByName("posNor");
		break;
	case VertexType_PosNorTex:
		if(mesh->texture != 0)
			technique = baseShader->GetTechniqueByName("posNorTex");
		else
			technique = baseShader->GetTechniqueByName("posNor");
		break;
	default:
		OutputDebugString(L"Could not draw indexed mesh, unrecognized vertex type!\n");
		return;
	}
	baseShader->SetTechnique(technique);

	D3DXMATRIX world;
	D3DXMATRIX temp;
	D3DXMatrixIdentity(&world);
	D3DXMatrixIdentity(&temp);
	world *= *D3DXMatrixRotationYawPitchRoll(&temp,mesh->rotationY,mesh->rotationX,mesh->rotationZ);
	world *= *D3DXMatrixTranslation(&temp,mesh->positionX,mesh->positionY,mesh->positionZ);

	baseShader->SetMatrix(baseShader->GetParameterByName(0,"world"),&world);
	D3DXMATRIX witMatrix; D3DXMatrixInverse(&witMatrix,0,&world); D3DXMatrixTranspose(&witMatrix,&witMatrix);
	baseShader->SetMatrix(baseShader->GetParameterByName(0,"worldInverseTranspose"), &witMatrix);
	baseShader->SetMatrix(baseShader->GetParameterByName(0,"worldViewProjection"), &(world*view*projection));

	SetShaderColor("materialColor",dx9mesh->colorR,dx9mesh->colorG,dx9mesh->colorB,dx9mesh->colorA);
	SetShaderVec3("cameraPosition", camera->x, camera->y, camera->z);
	SetShaderFloat("spotPower", 0.0f);
	SetShaderVec3("spotDirection", 0.0f, 0.0f, 0.0f);

	//D3DXHANDLE useLighting = baseShader->GetParameterByName(0, "useLighting"); // ????? !!!!!

	if(numLights > 0)
	{
		SetShaderFloat("ambient", 0.0f);
		
		SetShaderColor("lightColor",lights[0]->diffuseR,lights[0]->diffuseG,lights[0]->diffuseB, 1.0f);

		SetShaderFloat("specularPower", 16.0f);

		//D3DXHANDLE attenuation = baseShader->GetParameterByName(0, "attenuation");
		//D3DXVECTOR3 attenuationVector(1.0f,0.0f,0.0f);
		//baseShader->SetValue(attenuation, &attenuationVector, sizeof(D3DXVECTOR3));

		GpuLightType lightType = lights[0]->GetType();
		if(lightType == GpuLightType_Point)
		{
			GpuPointLight* pointLight = static_cast<GpuPointLight*>(lights[0]);
			SetShaderVec3("lightPosition", pointLight->x, pointLight->y, pointLight->z);
		}
		else if(lightType == GpuLightType_Spot)
		{
			GpuSpotLight* spotLight = static_cast<GpuSpotLight*>(lights[0]);
			SetShaderVec3("lightPosition", spotLight->x, spotLight->y, spotLight->z);
			SetShaderVec3("spotDirection", spotLight->u, spotLight->v, spotLight->w);
			SetShaderFloat("spotPower", spotLight->power);
		}
		else // GpuLightType::DIRECTIONALLIGHT
		{
			GpuDirectionalLight* dirLight = static_cast<GpuDirectionalLight*>(lights[0]); 
			//SetShaderVec3("spotDirection", dirLight->u, dirLight->v, dirLight->w);
			const float scale = 10000.f;
			SetShaderVec3("lightPosition", dirLight->u * scale, dirLight->v * scale, dirLight->w * scale);
		}
	}
	else
	{
		SetShaderVec3("lightPosition",0.0f,0.0f,0.0f);
		SetShaderColor("lightColor",1.0f,1.0f,1.0f,1.0f);
		SetShaderFloat("specularPower", FLT_MAX);
		SetShaderFloat("ambient", 1.0f);
	}

	if(dx9mesh->texture)
	{
		D3DXHANDLE tex = baseShader->GetParameterByName(0, "tex");
		DX9_GpuTexture* texture = static_cast<DX9_GpuTexture*>(mesh->texture);
		baseShader->SetTexture(tex, texture->textureObject);
	}

	baseShader->CommitChanges();

	if(wireFrame)
		direct3Ddevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	if(dx9mesh->wrapTexture)
		direct3Ddevice->SetRenderState(D3DRS_WRAP2, D3DWRAP_U);

	SetBufferState(buffer);
	
	unsigned numPasses = 1;
	baseShader->Begin(&numPasses,0);
	for(unsigned i = 0; i < numPasses; i++)
	{
		baseShader->BeginPass(i);
		direct3Ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 
			dx9mesh->numVertices, 0, dx9mesh->numTriangles);
		baseShader->EndPass();
	}
	baseShader->End();

	RestoreBufferState(buffer);

	if(wireFrame)
		direct3Ddevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	if(dx9mesh->wrapTexture)
		direct3Ddevice->SetRenderState(D3DRS_WRAP2, 0);
}
void DX9_GpuApi::DrawGpuScene(GpuScene* scene)
{

}

void DX9_GpuApi::LookTransform(
	float viewerX, float viewerY, float viewerZ, 
	float targetX, float targetY, float targetZ, 
	float upX, float upY, float upZ)
{
	//D3DXMATRIX view;
	D3DXMatrixLookAtLH(&view, 
		&D3DXVECTOR3(viewerX,viewerY,viewerZ), 
		&D3DXVECTOR3(targetX,targetY,targetZ),
		&D3DXVECTOR3(upX,upY,upZ));
	//direct3Ddevice->SetTransform(D3DTS_VIEW,&view);
}
void DX9_GpuApi::PerspectiveTransform(
	float fovY, float aspectRatio, 
	float zNear, float zFar)
{
	//D3DXMATRIX projection;
	D3DXMatrixPerspectiveFovLH(&projection,
		fovY, aspectRatio, zNear, zFar);
	//direct3Ddevice->SetTransform(D3DTS_PROJECTION,&projection);

}

void DX9_GpuApi::SetClearColor(float r, float g, float b)
{
	clearColor = floatToD3DColor(r,g,b,1.0f);
}

D3DCOLOR DX9_GpuApi::floatToD3DColor(float r, float g, float b, float a) const
{
	return D3DCOLOR_RGBA(static_cast<UINT>(r*255.f),static_cast<UINT>(g*255.f),
		static_cast<UINT>(b*255.f),static_cast<UINT>(a*255.f));
}

void DX9_GpuApi::SetScreenSize(int width, int height)
{
	presentParameters.BackBufferWidth = width;
	presentParameters.BackBufferHeight = height;
	onLostDevice();
	direct3Ddevice->Reset(&presentParameters);
	onResetDevice();
}

GpuFont* DX9_GpuApi::CreateGpuFont(int height, LPCWSTR facename, GpuFontStyle style)
{
	D3DXFONT_DESC fontDesc;
	fontDesc.Height          = height;
	fontDesc.Weight          = (style == GpuFontStyle_Bold) ? FW_BOLD : FW_NORMAL;
    fontDesc.MipLevels       = 1;
    fontDesc.Italic          = (style == GpuFontStyle_Italic);
    fontDesc.CharSet         = DEFAULT_CHARSET;
    fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
    fontDesc.Quality         = DEFAULT_QUALITY;
    fontDesc.PitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy_s(fontDesc.FaceName, facename);

	ID3DXFont * mFont;

	D3DXCreateFontIndirect(direct3Ddevice, &fontDesc, &mFont);

	if(mFont)
	{

		deviceDependentFonts.push_front(mFont);
		return new DX9_GpuFont(mFont);
	}
	else
	{
		// ERROR
		return 0;
	}
	
}

GpuTexture* DX9_GpuApi::CreateGpuTextureFromFile(LPCWSTR path){
	IDirect3DTexture9 * mTexture;

	HRESULT hr = D3DXCreateTextureFromFile(direct3Ddevice, path, &mTexture);

	if(SUCCEEDED(hr))
	{
		return new DX9_GpuTexture(mTexture);
	}
	else
	{
		OutputDebugString(L"Could not create texture from file.");
		return 0;
	}
}

DX9_GpuFont::~DX9_GpuFont()
{
	if(fontObject) fontObject->Release();
}

DX9_GpuTexture::~DX9_GpuTexture()
{
	if(textureObject) textureObject->Release();
}

bool DX9_GpuApi::isDeviceLost()
{
	HRESULT hr = direct3Ddevice->TestCooperativeLevel();

	switch(hr)
	{
	case D3DERR_DEVICELOST:
		return true;
	case D3DERR_DRIVERINTERNALERROR:
		// FATAL ERROR!!!!
		PostQuitMessage(0);
		return true;
	case D3DERR_DEVICENOTRESET:
		direct3Ddevice->Reset(&presentParameters);
		return false;
	default:
		return false;
	}
}
void DX9_GpuApi::onLostDevice()
{
	for(std::list<ID3DXFont*>::iterator i=deviceDependentFonts.begin(); 
		i != deviceDependentFonts.end(); ++i) 
	{
		static_cast<ID3DXFont*>(*i)->OnLostDevice();
	}
	if(spriteInterface) spriteInterface->OnLostDevice();
	if(baseShader) baseShader->OnLostDevice();
}
void DX9_GpuApi::onResetDevice()
{
	for(std::list<ID3DXFont*>::iterator i=deviceDependentFonts.begin(); 
		i != deviceDependentFonts.end(); ++i) 
	{
		static_cast<ID3DXFont*>(*i)->OnResetDevice();
	}
	if(spriteInterface) spriteInterface->OnResetDevice();
	if(baseShader) baseShader->OnResetDevice();
}

void DX9_GpuApi::SetBufferState(GpuDrawBuffer* buffer)
{
	if(buffer != 0)
	{
		switch (buffer->GetSpecial())
		{
		case GpuSpecialBuffer_Stencil:
			direct3Ddevice->SetRenderState(D3DRS_STENCILENABLE,    true);
			direct3Ddevice->SetRenderState(D3DRS_STENCILFUNC,      D3DCMP_ALWAYS);
			direct3Ddevice->SetRenderState(D3DRS_STENCILREF,       0x1);
			direct3Ddevice->SetRenderState(D3DRS_STENCILMASK,      0xffffffff);
			direct3Ddevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
			direct3Ddevice->SetRenderState(D3DRS_STENCILZFAIL,     D3DSTENCILOP_KEEP);
			direct3Ddevice->SetRenderState(D3DRS_STENCILFAIL,      D3DSTENCILOP_KEEP);
			direct3Ddevice->SetRenderState(D3DRS_STENCILPASS,      D3DSTENCILOP_REPLACE);
			direct3Ddevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
			break;
		case GpuSpecialBuffer_StencilClip:
			direct3Ddevice->SetRenderState(D3DRS_STENCILENABLE,    true);
			direct3Ddevice->SetRenderState(D3DRS_STENCILFUNC,  D3DCMP_EQUAL);
			direct3Ddevice->SetRenderState(D3DRS_STENCILREF,       0x1);
			direct3Ddevice->SetRenderState(D3DRS_STENCILMASK,      0xffffffff);
			direct3Ddevice->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);
			direct3Ddevice->SetRenderState(D3DRS_STENCILZFAIL,     D3DSTENCILOP_KEEP);
			direct3Ddevice->SetRenderState(D3DRS_STENCILFAIL,      D3DSTENCILOP_KEEP);
			direct3Ddevice->SetRenderState(D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP);
			direct3Ddevice->SetRenderState(D3DRS_ZENABLE, false);
			//gd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW)
			break;
		default:
			break;
		}
	}
}
void DX9_GpuApi::RestoreBufferState(GpuDrawBuffer* buffer)
{
	if(buffer != 0)
	{
		switch (buffer->GetSpecial())
		{
		case GpuSpecialBuffer_Stencil:
			direct3Ddevice->SetRenderState( D3DRS_ZWRITEENABLE, true );
			direct3Ddevice->SetRenderState( D3DRS_STENCILENABLE, false);
			break;
		case GpuSpecialBuffer_StencilClip:
			direct3Ddevice->SetRenderState( D3DRS_ZENABLE, true );
			direct3Ddevice->SetRenderState( D3DRS_STENCILENABLE, false);
		default:
			break;
		}
	}
}

GpuMesh* DX9_GpuApi::CreateGpuMesh(unsigned numVertices, unsigned size, 
								void* vertexBufferData, unsigned numTriangles)
{
	DX9_GpuMesh* createdMesh = new DX9_GpuMesh();
	
	direct3Ddevice->CreateVertexBuffer(
		size,
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_MANAGED,
		&createdMesh->vertexBuffer,
		0);

	void** data = 0;
	createdMesh->vertexBuffer->Lock(0, size, data, 0);
	memcpy(&data, &vertexBufferData, size);
	createdMesh->vertexBuffer->Unlock();

	createdMesh->numVertices = numVertices;

	return createdMesh;
}
GpuMesh* DX9_GpuApi::CreateGpuMesh(VertexBuffer* buffer, unsigned numTriangles)
{
	DX9_GpuMesh* createdMesh = new DX9_GpuMesh();
	unsigned size = buffer->GetElementSize() * buffer->GetLength();

	direct3Ddevice->CreateVertexBuffer(
		size,
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_MANAGED,
		&createdMesh->vertexBuffer,
		0);

	void** data = 0;
	createdMesh->vertexBuffer->Lock(0, size, data, 0);
	memcpy(&data, buffer->GetData(), size);
	createdMesh->vertexBuffer->Unlock();

	createdMesh->numVertices = buffer->GetLength();
	createdMesh->numTriangles = numTriangles;
	createdMesh->vertexDeclaration = DX9_VertApi::GetDeclaration(buffer->GetVertexType());
	createdMesh->vertexSize = buffer->GetElementSize();

	return createdMesh;
}
GpuIndexedMesh* DX9_GpuApi::CreateGpuIndexedMesh(
	unsigned numVertices, unsigned vertexDataSize, void* vertexData,
	unsigned numTriangles, unsigned indexDataSize, unsigned* indexData, VertexType type)
{
	DX9_GpuIndexedMesh* createdMesh = new DX9_GpuIndexedMesh();
	
	direct3Ddevice->CreateVertexBuffer(
		vertexDataSize,
		D3DUSAGE_WRITEONLY,
		0,
		D3DPOOL_MANAGED,
		&createdMesh->vertexBuffer,
		0);

	void* data = 0;
	createdMesh->vertexBuffer->Lock(0, 0, &data, 0);
	memcpy(data, vertexData, vertexDataSize);
	createdMesh->vertexBuffer->Unlock();

	createdMesh->vertexDeclaration = DX9_VertApi::GetDeclaration(type);
	createdMesh->vertexSize = DX9_VertApi::GetSize(type);

	direct3Ddevice->CreateIndexBuffer(
		indexDataSize,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX32,
		D3DPOOL_MANAGED,
		&createdMesh->indexBuffer,
		0);

	data = 0;
	createdMesh->indexBuffer->Lock(0, 0, &data, 0);
	memcpy(data, indexData, indexDataSize);
	createdMesh->indexBuffer->Unlock();

	createdMesh->numVertices = numVertices;
	createdMesh->numTriangles = numTriangles;
	createdMesh->vertexType = type;

	return createdMesh;
}
GpuIndexedMesh* DX9_GpuApi::CreateGpuIndexedMesh(VertexBuffer* buffer, unsigned numTriangles,
											  unsigned indexDataSize, unsigned *indexData)
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

GpuIndexedMesh* DX9_GpuApi::CreateTeapot()
{
	ID3DXMesh* teapotD3DXMesh;
	D3DXCreateTeapot(direct3Ddevice, &teapotD3DXMesh, 0);

	DX9_GpuIndexedMesh* createdMesh = D3DXToGpuIndexedMesh(teapotD3DXMesh);

	teapotD3DXMesh->Release();

	return createdMesh;
}
GpuIndexedMesh* DX9_GpuApi::CreateCube() { return 0; }
GpuIndexedMesh* DX9_GpuApi::CreateCylinder(
	float radius, float length, unsigned slices, unsigned stacks, bool texCoords)
{ 
	ID3DXMesh* cylinderD3DXMesh;
	D3DXCreateCylinder(direct3Ddevice,radius,radius,length,slices,stacks,&cylinderD3DXMesh,0);

	if(texCoords)
	{
		D3DVERTEXELEMENT9 elements[4];
		unsigned numElements = 0;
		DX9_VertApi::GetDeclaration(VertexType_PosNorTex)->GetDeclaration(elements,&numElements);

		ID3DXMesh* temp;
		cylinderD3DXMesh->CloneMesh(D3DXMESH_SYSTEMMEM,elements,direct3Ddevice,&temp);
		
		DX9Vertex_PosNorTex* vertices = 0;
		temp->LockVertexBuffer(0,(void**)&vertices);

		for(unsigned i = 0; i < temp->GetNumVertices(); i++)
		{
			D3DXVECTOR3 point = vertices[i].position;

			float theta = atan2f(point.y,point.x);
			float y2 = point.z + (length/2.0f);

			float u = theta / (2.0f * D3DX_PI);
			float v = y2 / length;

			//std::stringstream stream;
			//stream << u << "," << v << std::endl;
			//OutputDebugString(stream.str().c_str());

			vertices[i].texCoord.x = u;
			vertices[i].texCoord.y = v;
		}

		temp->UnlockVertexBuffer();

		cylinderD3DXMesh->Release();

		temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY, 
			elements, direct3Ddevice, &cylinderD3DXMesh);

		temp->Release();
	}

	DX9_GpuIndexedMesh* createdMesh = D3DXToGpuIndexedMesh(cylinderD3DXMesh);

	if(texCoords)
	{
		createdMesh->vertexDeclaration = DX9Vertex_PosNorTex::declaration;
		createdMesh->vertexSize = sizeof(DX9Vertex_PosNorTex);
		createdMesh->vertexType = VertexType_PosNorTex;
		createdMesh->wrapTexture = true;
	}

	cylinderD3DXMesh->Release();

	return createdMesh;
}
GpuIndexedMesh* DX9_GpuApi::CreateSphere(
	float radius, unsigned slices, unsigned stacks, bool texCoords) 
{  
	ID3DXMesh* sphereD3DXMesh;
	D3DXCreateSphere(direct3Ddevice,radius,slices,stacks,&sphereD3DXMesh,0);

	if(texCoords)
	{
		D3DVERTEXELEMENT9 elements[4];
		unsigned numElements = 0;
		DX9_VertApi::GetDeclaration(VertexType_PosNorTex)->GetDeclaration(elements,&numElements);

		ID3DXMesh* temp;
		sphereD3DXMesh->CloneMesh(D3DXMESH_SYSTEMMEM,elements,direct3Ddevice,&temp);
		
		DX9Vertex_PosNorTex* vertices = 0;
		temp->LockVertexBuffer(0,(void**)&vertices);

		for(unsigned i = 0; i < temp->GetNumVertices(); i++)
		{
			D3DXVECTOR3 point = vertices[i].position;

			float theta = atan2f(point.z,point.x);
			float phi   = acosf(point.y / sqrtf(
				(point.x*point.x) + (point.y*point.y) + (point.z*point.z)));

			float u = theta / (D3DX_PI * 2.0f);
			float v = phi / D3DX_PI;

			vertices[i].texCoord.x = u;
			vertices[i].texCoord.y = v;
		}

		temp->UnlockVertexBuffer();

		sphereD3DXMesh->Release();

		temp->CloneMesh(D3DXMESH_MANAGED | D3DXMESH_WRITEONLY, 
			elements, direct3Ddevice, &sphereD3DXMesh);

		temp->Release();
	}

	DX9_GpuIndexedMesh* createdMesh = D3DXToGpuIndexedMesh(sphereD3DXMesh);

	if(texCoords)
	{
		createdMesh->vertexDeclaration = DX9Vertex_PosNorTex::declaration;
		createdMesh->vertexSize = sizeof(DX9Vertex_PosNorTex);
		createdMesh->vertexType = VertexType_PosNorTex;
		createdMesh->wrapTexture = true;
	}

	sphereD3DXMesh->Release();

	return createdMesh;
}
GpuIndexedMesh* DX9_GpuApi::CreateGrid(float gridwidth, float griddepth, 
									   unsigned gridcolumns, unsigned gridrows,
									   GpuRect* textureRect) 
{ 
	IDirect3DVertexBuffer9* vertexBuffer;
	IDirect3DIndexBuffer9* indexBuffer;

	unsigned numVertices = gridcolumns * gridrows;

	if(textureRect)
	{
		direct3Ddevice->CreateVertexBuffer(numVertices * sizeof(DX9Vertex_PosNorTex), D3DUSAGE_WRITEONLY,
			0, D3DPOOL_MANAGED, &vertexBuffer, 0);
		DX9Vertex_PosNorTex* v = 0;
		vertexBuffer->Lock(0, 0, (void**)&v, 0);
		for(unsigned i = 0; i < gridrows; i++) {
			for(unsigned j = 0; j < gridcolumns; j++) {
				float x = (gridwidth * -.5f) + (j * (gridwidth/(gridcolumns - 1)));
				float z = (griddepth * -.5f) + (i * (griddepth/(gridrows - 1)));
				const float texRectWidth = textureRect->right - textureRect->left;
				const float texRectHeight = textureRect->bottom - textureRect->top;
				float tx = textureRect->left + (j * (texRectWidth/(gridcolumns - 1)));
				float ty = textureRect->top + (i * (texRectHeight/(gridrows - 1)));
				v[(gridcolumns * i) + j] = DX9Vertex_PosNorTex();
				v[(gridcolumns * i) + j].fill(x, 0.0f, z, 0.0f, 1.0f, 0.0f,tx,ty);
			}
		}
		vertexBuffer->Unlock();
	}
	else
	{
		direct3Ddevice->CreateVertexBuffer(numVertices * sizeof(DX9Vertex_PosNor), D3DUSAGE_WRITEONLY,
			0, D3DPOOL_MANAGED, &vertexBuffer, 0);
		DX9Vertex_PosNor* v = 0;
		vertexBuffer->Lock(0, 0, (void**)&v, 0);
		for(unsigned i = 0; i < gridrows; i++) {
			for(unsigned j = 0; j < gridcolumns; j++) {
				float x = (gridwidth * -.5f) + (j * (gridwidth/(gridcolumns - 1)));
				float z = (griddepth * -.5f) + (i * (griddepth/(gridrows - 1)));
				v[(gridcolumns * i) + j] = DX9Vertex_PosNor();
				v[(gridcolumns * i) + j].fill(x, 0.0f, z, 0.0f, 1.0f, 0.0f);
			}
		}
		vertexBuffer->Unlock();
	}

	unsigned numTriangles = (gridcolumns - 1) * (gridrows - 1) * 2;

	direct3Ddevice->CreateIndexBuffer(numTriangles * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &indexBuffer, 0);

	WORD* k = 0;

	indexBuffer->Lock(0, 0, (void**)&k, 0);

	int index = 0;

	// 0, n,   1; 1, n,   n+1; 
	// 1, n+1, 2; 2, n+1, n+2; winding order!

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

	indexBuffer->Unlock();

	DX9_GpuIndexedMesh* createdMesh = new DX9_GpuIndexedMesh();

	createdMesh->vertexBuffer = vertexBuffer;
	createdMesh->indexBuffer = indexBuffer;
	createdMesh->numVertices = numVertices;
	createdMesh->numTriangles = numTriangles;

	if(textureRect)
	{
		createdMesh->vertexDeclaration = DX9Vertex_PosNorTex::declaration;
		createdMesh->vertexSize = sizeof(DX9Vertex_PosNorTex);
		createdMesh->vertexType = VertexType_PosNorTex;
	}
	else
	{
		createdMesh->vertexDeclaration = DX9Vertex_PosNor::declaration;
		createdMesh->vertexSize = sizeof(DX9Vertex_PosNor);
		createdMesh->vertexType = VertexType_PosNor;
	}
	
	return createdMesh;
}

float DX9_GpuApi::MeasureGpuText(GpuFont* font, const wchar_t* text)
{
	//DX9_GpuFont* dx9font = static_cast<DX9_GpuFont*>(font);
	//TEXTMETRICW textMetric = {0};
	//dx9font->fontObject->GetTextMetricsW(&textMetric);
	return 0.0f;
}

GpuRect DX9_GpuApi::GetTextureDimensions(GpuTexture *texture)
{
	DX9_GpuTexture* dx9texture = static_cast<DX9_GpuTexture*>(texture);
	return GpuRect(0.0f, 0.0f, 
		static_cast<float>(dx9texture->textureObject->Width),
		static_cast<float>(dx9texture->textureObject->Height));
}

DX9_GpuIndexedMesh* DX9_GpuApi::D3DXToGpuIndexedMesh(ID3DXMesh* d3dxmesh)
{
	IDirect3DVertexBuffer9* vertexBuffer;
	IDirect3DIndexBuffer9* indexBuffer;

	d3dxmesh->GetVertexBuffer(&vertexBuffer);
	d3dxmesh->GetIndexBuffer(&indexBuffer);

	DX9_GpuIndexedMesh* createdMesh = new DX9_GpuIndexedMesh();

	createdMesh->vertexBuffer = vertexBuffer;
	createdMesh->indexBuffer = indexBuffer;

	createdMesh->numVertices = d3dxmesh->GetNumVertices();
	createdMesh->numTriangles = d3dxmesh->GetNumFaces();

	createdMesh->vertexDeclaration = DX9Vertex_PosNor::declaration;
	createdMesh->vertexSize = sizeof(DX9Vertex_PosNor);
	createdMesh->vertexType = VertexType_PosNor;
	
	return createdMesh;
}

DX9_GpuMesh::~DX9_GpuMesh()
{
	if(vertexBuffer) vertexBuffer->Release(); vertexDeclaration = 0;
	//if(vertexDeclaration) vertexDeclaration->Release(); vertexDeclaration = 0;
}

DX9_GpuIndexedMesh::~DX9_GpuIndexedMesh()
{
	if(vertexBuffer) vertexBuffer->Release(); vertexBuffer = 0;
	if(indexBuffer) indexBuffer->Release(); indexBuffer = 0;
	//if(vertexDeclaration) vertexDeclaration->Release(); vertexDeclaration = 0;
}

#endif
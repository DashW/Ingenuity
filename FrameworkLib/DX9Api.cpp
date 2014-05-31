#include "stdafx.h"

#ifdef USE_DX9_GPUAPI

#include "DX9Shaders.h"
#include "DX9Surfaces.h"
#include "GeoBuilder.h"
#include "ShaderParser.h"
#include "tinyxml2.h"
#include <windef.h>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <glm/ext.hpp>

DX9_GpuApi::DX9_GpuApi(FileApi * files, StepMgr * steppables, HWND windowHandle) : 
	GpuApi(),
	direct3D(0),
	direct3Ddevice(0),
	spriteInterface(0),
	texShaderDeclaration(0),
	view(),
	projection(),
	presentParameters(),
	files(files),
	steppables(steppables),
	clearColor(0xffffffff),
	//clearColor(0x00000000),
	renderStateWireframe(false)
{
	direct3D = Direct3DCreate9(D3D_SDK_VERSION);
	if( !direct3D )
	{
		//initialised = false;
		return;
	}

	D3DDEVTYPE devType = D3DDEVTYPE_HAL;

	// Check device capabilities

	D3DDISPLAYMODE mode;
	direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	direct3D->CheckDeviceType(D3DADAPTER_DEFAULT, devType, mode.Format, mode.Format, true);
	direct3D->CheckDeviceType(D3DADAPTER_DEFAULT, devType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false);
	
	D3DCAPS9 caps;
	direct3D->GetDeviceCaps(D3DADAPTER_DEFAULT, devType, &caps);

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
	presentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
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
	//presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// Create Device

	direct3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		devType,
		windowHandle, 
		devBehaviorFlags, 
		&presentParameters,
		&direct3Ddevice);

	SetupVertexInformation();

	D3DXCreateSprite(direct3Ddevice,&spriteInterface);

	//baseShader = static_cast<DX9_ModelShader*>(GetShader("BaseShader"));
	//texCopyShader = static_cast<DX9_TextureShader*>(GetShader("TextureCopy"));

	LocalMesh * tempQuad = GeoBuilder().BuildRect(0.0f,0.0f,1.0f,1.0f,true);
	texShaderQuad = static_cast<DX9_GpuMesh*>(tempQuad->GpuOnly(this));

	stencilSurface = new DX9_GpuStencilSurface(direct3Ddevice);
	stencilClipSurface = new DX9_GpuStencilClipSurface(direct3Ddevice);

	//initialised = true;
}

DX9_GpuApi::~DX9_GpuApi()
{
	if(texShaderDeclaration) texShaderDeclaration->Release();
	if(stencilSurface) delete stencilSurface;
	if(stencilClipSurface) delete stencilClipSurface;
	if(texShaderQuad) delete texShaderQuad;

	if(spriteInterface) spriteInterface->Release();

	if(direct3Ddevice) direct3Ddevice->Release(); direct3Ddevice = 0;
	if(direct3D) direct3D->Release(); direct3D = 0;
}

struct DX9_GpuApi::ShaderLoader : public GpuShaderLoader
{
	DX9_GpuApi * gpu;

	std::vector<const char*> techniqueNames;
	std::vector<VertexType> vertexTypes;
	std::vector<InstanceType> instanceTypes;

	std::vector<DX9_GpuShader::ParamMapping> paramMappings;

	bool xmlParsed;
	bool shaderFileOpened;
	bool shaderFileResponded;

	ShaderLoader(FileApi * files, DX9_GpuApi * gpu, FileApi_Directory * directory, const wchar_t * path) :
		GpuShaderLoader(files, directory, path), gpu(gpu), xmlParsed(false), shaderFileOpened(false), shaderFileResponded(false) {}
	virtual ~ShaderLoader() {}
	
	struct D3DXEffectResponse : public FileApi_Response
	{
		ShaderLoader * parent;
		ShaderParser * parser;

		D3DXEffectResponse(ShaderLoader * parent, ShaderParser * parser) :
			parent(parent), parser(parser) {}

		virtual void Respond() override
		{
			closeOnComplete = true; deleteOnComplete = true;

			if(buffer)
			{
				ID3DXEffect * effectObject = 0;
				ID3DXBuffer * errors = 0;

				D3DXCreateEffect(parent->gpu->direct3Ddevice, buffer, bufferLength, 0, 0, D3DXSHADER_DEBUG,
					0, &effectObject, &errors);

				if(effectObject)
				{
					DX9_GpuShader * shader = 0;

					if(parser->IsModelShader())
					{
						DX9_ModelShader * modelShader = new DX9_ModelShader(parent->gpu,effectObject);

						for(unsigned i = 0; i < parent->techniqueNames.size(); ++i)
						{
							D3DXHANDLE handle = modelShader->shaderObject->GetTechniqueByName(parent->techniqueNames[i]);
							if(handle)
							{
								VertexType vertexType = parent->vertexTypes[i];
								InstanceType instanceType = parent->instanceTypes[i];
								unsigned key = VertApi::GetTechniqueKey(vertexType, instanceType);
								modelShader->techniques[key].handle = handle;

								std::vector<D3DVERTEXELEMENT9> elements;
								D3DVERTEXELEMENT9 end = D3DDECL_END();
								D3DVERTEXELEMENT9 * vertexElems = parent->gpu->vertexElements[vertexType];
								for(unsigned i = 0; vertexElems[i].Type != D3DDECLTYPE_UNUSED; ++i)
								{
									elements.push_back(vertexElems[i]);
								}
								D3DVERTEXELEMENT9 * instanceElems = parent->gpu->instanceElements[instanceType];
								for(unsigned i = 0; instanceElems[i].Type != D3DDECLTYPE_UNUSED; ++i)
								{
									elements.push_back(instanceElems[i]);
								}
								elements.push_back(end);

								parent->gpu->direct3Ddevice->CreateVertexDeclaration(elements.data(),
									&(modelShader->techniques[key].declaration));
							}
						}
						
						shader = modelShader;
					}
					else
					{
						shader = new DX9_TextureShader(parent->gpu, effectObject);
					}

					for(unsigned i = 0; i < parser->GetNumParams(); ++i)
					{
						shader->paramSpecs.push_back(parser->GetParamSpec(i));
					}

					for(unsigned i = 0; i < parent->paramMappings.size(); ++i)
					{
						DX9_GpuShader::ParamMapping& mapping = parent->paramMappings[i];
						mapping.handle = shader->shaderObject->GetParameterByName(0, mapping.paramName.c_str());
						if(mapping.handle)
						{
							shader->paramMappings.push_back(mapping);
						}
					}

					parent->asset = shader;
				}
				else
				{
					OutputDebugString(L"Failed to load shader ");
					OutputDebugString(parent->path.c_str());
					OutputDebugString(L"\n");
					if(errors)
					{
						OutputDebugString((LPCWSTR)errors->GetBufferPointer());
						OutputDebugString(L"\n");
					}
				}
			}

			parent->shaderFileResponded = true;
		}
	};

	virtual void ParseXml() override
	{
		tinyxml2::XMLElement * targetApiElement = parser->GetApiElement("directx9");
		if(targetApiElement)
		{
			if(parser->IsModelShader())
			{
				tinyxml2::XMLElement * techniqueElement = targetApiElement->FirstChildElement("technique");
				while(techniqueElement)
				{
					const char * techniqueNameChars = techniqueElement->Attribute("name");

					VertexType vertexType = VertexType_Pos;
					const char * vertexTypeChars = techniqueElement->Attribute("vertexType");
					if(vertexTypeChars)
					{
						for(unsigned i = 0; i < VertexType_Count; ++i)
						{
							if(strcmp(vertexTypeChars, VertApi::GetVertexName((VertexType)i)) == 0)
							{
								vertexType = (VertexType)i;
								break;
							}
						}
					}

					InstanceType instanceType = InstanceType_None;
					const char * instanceTypeChars = techniqueElement->Attribute("instanceType");
					if(instanceTypeChars)
					{
						if(strcmp(instanceTypeChars, "Pos") == 0)
						{
							instanceType = InstanceType_Pos;
						}
						if(strcmp(instanceTypeChars, "PosCol") == 0)
						{
							instanceType = InstanceType_PosCol;
						}
					}

					if(techniqueNameChars)
					{
						techniqueNames.push_back(techniqueNameChars);
						vertexTypes.push_back(vertexType);
						instanceTypes.push_back(instanceType);
					}

					techniqueElement = techniqueElement->NextSiblingElement("technique");
				}
			}

			tinyxml2::XMLElement * mappingElement = targetApiElement->FirstChildElement("paramMapping");
			while(mappingElement)
			{
				DX9_GpuShader::ParamMapping mapping;
				mapping.paramIndex = mappingElement->UnsignedAttribute("index");

				if(mapping.paramIndex >= parser->GetNumParams())
				{
					OutputDebugString(L"Model shader param mapping has an invalid index!\n");
					shaderFileOpened = true;
					return;
				}

				const char * paramNameChars = mappingElement->Attribute("name");
				if(paramNameChars)
				{
					mapping.paramName = paramNameChars;
				}

				paramMappings.push_back(mapping);

				mappingElement = mappingElement->NextSiblingElement("paramMapping");
			}

			const char * shaderName = targetApiElement->Attribute("shader");
			if(shaderName)
			{
				std::string shortName(shaderName);
				std::wstring wideName(shortName.begin(), shortName.end());
				wideName += L".cso";

				files->OpenAndRead(directory, wideName.c_str(), new D3DXEffectResponse(this, parser));
				shaderFileOpened = true;
			}
		}

		xmlParsed = true;
	}

	virtual bool HasParsedXml() override
	{
		return xmlParsed && (shaderFileOpened == shaderFileResponded);
	}
	
};

void DX9_GpuApi::Initialize(AssetMgr * assets)
{
	// Load the default shaders

	FileApi_Directory * frameworkDir = files->GetKnownDirectory(FileApi::FrameworkDir);

	assets->Load(frameworkDir, L"BaseShader.xml", ShaderAsset, 0, AssetMgr::CRITICAL_TICKET);
	assets->Load(frameworkDir, L"TextureCopy.xml", ShaderAsset, 0, AssetMgr::CRITICAL_TICKET);
}

void DX9_GpuApi::OnCriticalLoad(AssetMgr * assets)
{
	FileApi_Directory * frameworkDir = files->GetKnownDirectory(FileApi::FrameworkDir);

	GpuShader * gpuBaseShader = assets->GetAsset<GpuShader>(frameworkDir, L"BaseShader.xml");
	GpuShader * gpuTexCopyShader = assets->GetAsset<GpuShader>(frameworkDir, L"TextureCopy.xml");

	baseShader = static_cast<DX9_ModelShader*>(gpuBaseShader);
	texCopyShader = static_cast<DX9_TextureShader*>(gpuTexCopyShader);
}

void DX9_GpuApi::Clear()
{
	direct3Ddevice->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0);
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

void DX9_GpuApi::DrawGpuText(GpuFont* font, LPCWSTR text, float x, float y, bool center, GpuDrawSurface * surface)
{
	if(!font) return;

	DX9_GpuFont * dxfont = static_cast<DX9_GpuFont*>(font);
	RECT rect = {0};

	float screenWidth = float(presentParameters.BackBufferWidth);
	float screenHeight = float(presentParameters.BackBufferHeight);

	if(!font->pixelSpace)
	{
		x = (screenWidth + (x * screenHeight)) / 2.0f;
		y = (screenHeight + (y * screenHeight)) / 2.0f;
	}

	rect.left = (long) x; rect.top = long(y);
	dxfont->fontObject->DrawTextW(0, text, -1, &rect, DT_CALCRECT, 
		floatToD3DColor(1.0,1.0,1.0,1.0));

	// rect could be used as a CLIPPING REGION!

	D3DXMATRIX transform;
	D3DXMATRIX intermediate;
	transform = *D3DXMatrixIdentity(&intermediate);

	spriteInterface->SetTransform(&transform);
	
	if(center)
	{
		transform *= *D3DXMatrixTranslation(&intermediate, 
			- float((rect.right - x) / 2), - float((rect.bottom - y) / 2), 0.0f);
	}

	transform *= *D3DXMatrixTranslation(&intermediate, -x, -y, 0.0f);

	float scaleFactor = dxfont->size;

	if(!font->pixelSpace)
	{
		scaleFactor *= float(presentParameters.BackBufferHeight) / float(standardScreenHeight);
	}

	transform *= *D3DXMatrixScaling(&intermediate, scaleFactor, scaleFactor, 1.0f);

	transform *= *D3DXMatrixTranslation(&intermediate, x, y, 0.0f);

	spriteInterface->SetTransform(&transform);
	spriteInterface->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
	dxfont->fontObject->DrawTextW(spriteInterface, text, -1, &rect, DT_LEFT | DT_TOP, 
		floatToD3DColor(dxfont->color.r,dxfont->color.g,dxfont->color.b,dxfont->color.a));
	spriteInterface->Flush();
	spriteInterface->End();
}

void DX9_GpuApi::DrawGpuSprite(GpuSprite* sprite, GpuDrawSurface * surface)
{
	if(!spriteInterface || !sprite) return;

	// Alpha test and reference value.
	direct3Ddevice->SetRenderState(D3DRS_ALPHAREF, 10);
	direct3Ddevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	if(sprite->brightAsAlpha)
	{
		direct3Ddevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_ADD);
		direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	}
	else
	{
		direct3Ddevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		direct3Ddevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
		direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	//direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	direct3Ddevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

	DX9_GpuTexture* texture = static_cast<DX9_GpuTexture*>(sprite->texture);
	if(!texture || !texture->textureObject) return;

	D3DXVECTOR3 center(sprite->center.x,sprite->center.y,0.0f);
	D3DXVECTOR3 position(0.0f,0.0f,0.0f);
	position.z = sprite->position.z;
	float w = static_cast<float>(presentParameters.BackBufferWidth/2);
	float h = static_cast<float>(presentParameters.BackBufferHeight/2);
	if(sprite->pixelSpace)
	{
		position.x = sprite->position.x; 
		position.y = sprite->position.y; 
	}
	else
	{
		position.x = w + (sprite->position.x * h);
		position.y = h + (sprite->position.y * h);
	}

	D3DXMATRIX centerMatrix, translationMatrix, rotationMatrix, scaleMatrix, screenMatrix, transformMatrix;

	D3DXMatrixTranslation(&centerMatrix,-center.x,-center.y,0.0f);
	D3DXMatrixTranslation(&translationMatrix, position.x, position.y, position.z);
	D3DXMatrixRotationZ(&rotationMatrix, sprite->rotation);
	D3DXMatrixScaling(&scaleMatrix, sprite->size * sprite->scale.x, sprite->size * sprite->scale.y, 1.0f);
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

	DX9_GpuTexture * dx9tex = (DX9_GpuTexture*) sprite->texture;

	RECT sourceRect;
	sourceRect.left   =	(LONG) (dx9tex->width  * sprite->clipRect.left);
	sourceRect.top    =	(LONG) (dx9tex->height * sprite->clipRect.top);
	sourceRect.right  =	(LONG) (dx9tex->width  * sprite->clipRect.right);
	sourceRect.bottom = (LONG) (dx9tex->height * sprite->clipRect.bottom);

	D3DXCOLOR colorization(sprite->color.r, sprite->color.g, sprite->color.b, sprite->color.a);

	if(surface) ((DX9_GpuDrawSurface*)surface)->Begin();

	spriteInterface->Begin(D3DXSPRITE_ALPHABLEND);
	if(FAILED(spriteInterface->Draw(texture->textureObject,&sourceRect,0,0,colorization)))
	{
		OutputDebugString(L"FAILED!!!");
	}
	spriteInterface->Flush();
	spriteInterface->End();

	if(surface) ((DX9_GpuDrawSurface*)surface)->End();

	//direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	direct3Ddevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
}

void DX9_GpuApi::DrawGpuModel(GpuModel * model, GpuCamera * camera, GpuLight ** lights, 
								 unsigned numLights, GpuDrawSurface * surface, GpuInstanceBuffer * instances)
{
	if(!model || !model->mesh) return;
	if(model->backFaceCull && model->frontFaceCull) return; // lol :P

	DX9_GpuMesh* dx9mesh = static_cast<DX9_GpuMesh*>(model->mesh);

	VertexType vertexType = dx9mesh->vertexType;
	InstanceType instanceType = InstanceType_None;

	direct3Ddevice->SetStreamSource(0, dx9mesh->vertexBuffer, 0, dx9mesh->vertexSize);

	if(instances)
	{
		DX9_GpuInstanceBuffer * dx9instanceBuffer = static_cast<DX9_GpuInstanceBuffer*>(instances);

		instanceType = dx9instanceBuffer->type;

		direct3Ddevice->SetStreamSource(1, dx9instanceBuffer->buffer, 0, dx9instanceBuffer->instanceSize);

		direct3Ddevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | dx9instanceBuffer->numInstances);
		direct3Ddevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1u);
	}
	else
	{
		direct3Ddevice->SetStreamSourceFreq(0, 1);
		direct3Ddevice->SetStreamSourceFreq(1, 1); // 1, 0 ?
	}
	
	direct3Ddevice->SetIndices(dx9mesh->indexBuffer);

	if(!baseShader) return;
	DX9_ModelShader * shader = baseShader;
	if(model->effect && model->effect->shader && model->effect->shader->IsModelShader()) 
	{
		shader = static_cast<DX9_ModelShader*>(model->effect->shader);
	}

	if(!shader->SetTechnique(vertexType, instanceType))
	{
		OutputDebugString(L"Could not draw mesh, incompatible vertex type!\n");
		return;
	}

	unsigned techniqueKey = VertApi::GetTechniqueKey(vertexType, instanceType);
	direct3Ddevice->SetVertexDeclaration(shader->techniques[techniqueKey].declaration);

	float aspect = float(presentParameters.BackBufferWidth)/float(presentParameters.BackBufferHeight);
	if(surface)
	{
		GpuDrawSurface::Type surfaceType = surface->GetSurfaceType();
		if(surfaceType == GpuDrawSurface::Texture || surfaceType == GpuDrawSurface::FullscreenTexture)
		{
			DX9_GpuTextureSurface * texSurface = static_cast<DX9_GpuTextureSurface*>(surface);
			aspect = float(texSurface->viewport.Width)/float(texSurface->viewport.Height);
			texSurface->viewport.MinZ = camera->farClip;
			texSurface->viewport.MaxZ = camera->nearClip; // Surely this should be the other way round?
		}
	}
	shader->SetParameters(model, camera, lights, numLights, aspect);

	if(model->effect) 
		ApplySamplerParameters(model->effect);

	bool shouldDrawWireframe = drawEverythingWireframe || model->wireframe;
	if(renderStateWireframe != shouldDrawWireframe)
	{
		direct3Ddevice->SetRenderState(D3DRS_FILLMODE, shouldDrawWireframe ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
		renderStateWireframe = shouldDrawWireframe;
	}

	if(model->wrapTexture)
		direct3Ddevice->SetRenderState(D3DRS_WRAP2, D3DWRAP_U);
	if(!model->backFaceCull)
	{
		if(model->frontFaceCull)
			direct3Ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		else
			direct3Ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}

	direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

	if(surface) ((DX9_GpuDrawSurface*)surface)->Begin();
	
	unsigned numPasses = 1;
	shader->shaderObject->Begin(&numPasses,0);
	for(unsigned i = 0; i < numPasses; i++)
	{
		shader->shaderObject->BeginPass(i);

		if(dx9mesh->IsIndexed())
		{
			direct3Ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 
				dx9mesh->numVertices, 0, dx9mesh->numTriangles);
		}
		else
		{
			direct3Ddevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 
				dx9mesh->numVertices / 3);
		}

		shader->shaderObject->EndPass();
	}
	shader->shaderObject->End();

	if(surface) ((DX9_GpuDrawSurface*)surface)->End();

	//direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	if(model->wrapTexture)
		direct3Ddevice->SetRenderState(D3DRS_WRAP2, 0);
	if(!model->backFaceCull)
		direct3Ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void DX9_GpuApi::DrawGpuSurface(GpuDrawSurface * source, GpuEffect * effect, GpuDrawSurface * dest)
{
	if(!(source && effect && dest)) return;
	
	DX9_GpuDrawSurface * dx9source = static_cast<DX9_GpuDrawSurface*>(source);
	DX9_GpuTexture * dx9tex = static_cast<DX9_GpuTexture*>(dx9source->GetTexture());

	direct3Ddevice->SetStreamSource(0, texShaderQuad->vertexBuffer, 0, texShaderQuad->vertexSize);
	direct3Ddevice->SetVertexDeclaration(texShaderDeclaration);
	direct3Ddevice->SetIndices(texShaderQuad->indexBuffer);

	DX9_TextureShader * texShader = static_cast<DX9_TextureShader*>(effect->shader);
	if(!texShader->IsTextureShader()) return;
	texShader->SetParameters(dx9tex,effect);
	texShader->shaderObject->CommitChanges();

	if(dest->GetSurfaceType() == GpuDrawSurface::Stencil 
		|| dest->GetSurfaceType() == GpuDrawSurface::StencilClip) return;
	DX9_GpuTextureSurface * texSurface = static_cast<DX9_GpuTextureSurface*>(dest);
	if(texSurface->viewport.Width != dx9tex->width 
		|| texSurface->viewport.Height != dx9tex->height) return;

	direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	if(effect->shader->blendMode == GpuShader::Max)
	{
		direct3Ddevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		direct3Ddevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);
		direct3Ddevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);
	}
	direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

	texSurface->Begin();
	//direct3Ddevice->Clear(0,0,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0);
	texShader->shaderObject->Begin(0,0);
	texShader->shaderObject->BeginPass(0);
	if(FAILED(direct3Ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 
		texShaderQuad->numVertices, 0, texShaderQuad->numTriangles)))
	{
		OutputDebugString(L"Failed!!!");
	}
	texShader->shaderObject->EndPass();
	texShader->shaderObject->End();
	texSurface->End();

	direct3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	direct3Ddevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

void DX9_GpuApi::SetClearColor(float r, float g, float b, float a)
{
	clearColor = floatToD3DColor(r,g,b,a);
}

D3DCOLOR DX9_GpuApi::floatToD3DColor(float r, float g, float b, float a) const
{
	return D3DCOLOR_RGBA(static_cast<UINT>(r*255.f),static_cast<UINT>(g*255.f),
		static_cast<UINT>(b*255.f),static_cast<UINT>(a*255.f));
}

void DX9_GpuApi::ApplySamplerParameters(GpuEffect * effect)
{
	static const D3DSAMPLERSTATETYPE GPU_SAMPLER_KEYS_TO_DX9_STATE_TYPES[] = {
		D3DSAMP_ADDRESSU,
		D3DSAMP_ADDRESSV,
		D3DSAMP_ADDRESSW,
		D3DSAMP_MINFILTER,
		D3DSAMP_MAGFILTER,
		D3DSAMP_MIPFILTER,
		D3DSAMP_MAXANISOTROPY
	};

	for(unsigned i = 0; i < effect->samplerParams.size(); ++i)
	{
		GpuSamplerParam & param = effect->samplerParams[i];
		D3DSAMPLERSTATETYPE dx9key;
		unsigned dx9value;

		dx9key = GPU_SAMPLER_KEYS_TO_DX9_STATE_TYPES[param.key];

		switch(param.key)
		{
		case GpuSamplerParam::AddressU:
		case GpuSamplerParam::AddressV:
		case GpuSamplerParam::AddressW:
			switch(param.value)
			{
			case GpuSamplerParam::AddressWrap:
				dx9value = D3DTADDRESS_WRAP;
				break;
			case GpuSamplerParam::AddressMirror:
				dx9value = D3DTADDRESS_MIRROR;
				break;
			case GpuSamplerParam::AddressClamp:
				dx9value = D3DTADDRESS_CLAMP;
				break;
			}
			break;
		case GpuSamplerParam::FilterMin:
		case GpuSamplerParam::FilterMag:
		case GpuSamplerParam::FilterMip:
			switch(param.value)
			{
			case GpuSamplerParam::FilterNone:
				dx9value = D3DTEXF_NONE;
				break;
			case GpuSamplerParam::FilterPoint:
				dx9value = D3DTEXF_POINT;
				break;
			case GpuSamplerParam::FilterLinear:
				dx9value = D3DTEXF_LINEAR;
				break;
			case GpuSamplerParam::FilterAnisotropic:
				dx9value = D3DTEXF_ANISOTROPIC;
				break;
			}
			break;
		default:
			dx9value = param.value;
		}

		direct3Ddevice->SetSamplerState(param.samplerIndex, dx9key, dx9value);
	}
}

D3DVERTEXELEMENT9 pos[] = {
	{0, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	D3DDECL_END()
};	
D3DVERTEXELEMENT9 posCol[] = {
	{0, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
	D3DDECL_END()
};
D3DVERTEXELEMENT9 posNor[] = {
	{0, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	D3DDECL_END()
};
D3DVERTEXELEMENT9 posTex[] = {
	{0, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12,  D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};
D3DVERTEXELEMENT9 posNorTex[] = {
	{0, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	{0, 24,  D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};
D3DVERTEXELEMENT9 posNorTanTex[] = {
	{0, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	{0, 24,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0},
	{0, 40,  D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};

// Instanced
D3DVERTEXELEMENT9 instNone[] = {
	D3DDECL_END()
};
D3DVERTEXELEMENT9 instPos[] = {
	{1, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
	D3DDECL_END()
};
D3DVERTEXELEMENT9 instPosCol[] = {
	{1, 0,   D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
	{1, 12,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    1},
	D3DDECL_END()
};

void DX9_GpuApi::SetupVertexInformation()
{
	for(unsigned i = 0; i < VertexType_Count; ++i) 
	{ 
		vertexElements[i] = 0; 
	}
	for(unsigned i = 0; i < InstanceType_Count; ++i)
	{
		instanceElements[i] = 0;
	}

	vertexElements[VertexType_Pos] = pos;
	vertexElements[VertexType_PosCol] = posCol;
	vertexElements[VertexType_PosNor] = posNor;
	vertexElements[VertexType_PosTex] = posTex;
	vertexElements[VertexType_PosNorTex] = posNorTex;
	vertexElements[VertexType_PosNorTanTex] = posNorTanTex;

	instanceElements[InstanceType_None] = instNone;
	instanceElements[InstanceType_Pos] = instPos;
	instanceElements[InstanceType_PosCol] = instPosCol;

	if(!texShaderDeclaration)
	{
		direct3Ddevice->CreateVertexDeclaration(posTex,&texShaderDeclaration);
	}
}

void DX9_GpuApi::OnScreenResize(unsigned width, unsigned height)
{
	presentParameters.BackBufferWidth = width;
	presentParameters.BackBufferHeight = height;
	OnLostDevice();
	if(FAILED(direct3Ddevice->Reset(&presentParameters)))
	{
		OutputDebugString(L"Uh oh! Did you release all your resources???\n");
		if(IsDebuggerPresent()) DebugBreak();
	}
	OnResetDevice();
}

void DX9_GpuApi::GetBackbufferSize(unsigned & width, unsigned & height)
{
	width = presentParameters.BackBufferWidth;
	height = presentParameters.BackBufferHeight;
}

void DX9_GpuApi::SetMultisampling(unsigned multisampleLevel)
{
	if(multisampleLevel < 17 && presentParameters.MultiSampleType != multisampleLevel)
	{
		presentParameters.MultiSampleType = (D3DMULTISAMPLE_TYPE) multisampleLevel;
		OnLostDevice();
		if(FAILED(direct3Ddevice->Reset(&presentParameters)))
		{
			OutputDebugString(L"Uh oh! Did you release all your resources???");
			if(IsDebuggerPresent()) DebugBreak();
		}
		OnResetDevice();
	}
}

GpuFont* DX9_GpuApi::CreateGpuFont(int height, LPCWSTR facename, GpuFontStyle style)
{
	D3DXFONT_DESC fontDesc = {0};
	fontDesc.Height          = 62;
	fontDesc.Weight          = (style == GpuFontStyle_Bold) ? FW_BOLD : FW_NORMAL;
    fontDesc.MipLevels       = 1;
    fontDesc.Italic          = (style == GpuFontStyle_Italic);
    fontDesc.CharSet         = DEFAULT_CHARSET;
    fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
    fontDesc.Quality         = DEFAULT_QUALITY;
    fontDesc.PitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
	fontDesc.Width			 = 0;
	wcscpy_s(fontDesc.FaceName, facename);

	ID3DXFont * mFont = 0;

	D3DXCreateFontIndirect(direct3Ddevice, &fontDesc, &mFont);

	if(mFont)
	{
		DX9_GpuFont * font = new DX9_GpuFont(this,mFont);
		font->size = float(height) / 64.0f;
		return font;
	}
	// ERROR
	return 0;
}

GpuTexture * DX9_GpuApi::CreateGpuTexture(char * data, unsigned dataSize, bool isDDS){
	IDirect3DTexture9 * mTexture;

	D3DXIMAGE_INFO info;

	//HRESULT hr = D3DXCreateTextureFromFile(direct3Ddevice,path,&mTexture);
	//HRESULT hr = D3DXCreateTextureFromFileEx(direct3Ddevice, filePath.c_str(), 
	//	D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT, 0, 
	//	D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
	//	D3DX_DEFAULT, D3DX_DEFAULT, 
	//	0, &info, 0, &mTexture);

	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(direct3Ddevice, data, dataSize,
		D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, 
		D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, 0, &mTexture);

	if(SUCCEEDED(hr))
	{
		DX9_GpuTexture * result = new DX9_GpuTexture(mTexture,info.Width,info.Height);
		//result->texPath = path;
		return result;
	}
	else
	{
		OutputDebugString(L"Could not create texture\n");
		return 0;
	}
}

GpuCubeMap * DX9_GpuApi::CreateGpuCubeMap(char * data, unsigned dataSize){
	IDirect3DCubeTexture9* mTexture;

	//HRESULT hr = D3DXCreateCubeTextureFromFile(direct3Ddevice, filePath.c_str(), &mTexture);

	HRESULT hr = D3DXCreateCubeTextureFromFileInMemory(direct3Ddevice, data, dataSize, &mTexture);

	if(SUCCEEDED(hr))
	{
		return new DX9_GpuCubeMap(mTexture);
	}
	else
	{
		OutputDebugString(L"Could not create cube map\n");
		return 0;
	}
}

GpuVolumeTexture * DX9_GpuApi::CreateGpuVolumeTexture(char * data, unsigned dataSize)
{
	IDirect3DVolumeTexture9 * mTexture;

	//HRESULT hr = D3DXCreateVolumeTextureFromFile(direct3Ddevice, filePath.c_str(), &mTexture);

	HRESULT hr = D3DXCreateVolumeTextureFromFileInMemory(direct3Ddevice, data, dataSize, &mTexture);

	if(SUCCEEDED(hr))
	{
		return new DX9_GpuVolumeTexture(mTexture);
	}
	else
	{
		OutputDebugString(L"Could not create volume texture\n");
		return 0;
	}
}

GpuShaderLoader * DX9_GpuApi::CreateGpuShaderLoader(FileApi_Directory * directory, const wchar_t * path)
{
	return new ShaderLoader(files, this, directory, path);
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
		OutputDebugString(L"Driver internal error!\n");
		PostQuitMessage(0);
		return true;
	case D3DERR_DEVICENOTRESET:
		hr = direct3Ddevice->Reset(&presentParameters);
		if(!SUCCEEDED(hr))
		{
			OutputDebugString(L"Device reset failed!\n");
			PostQuitMessage(0);
			return true;
		}
		return false;
	default:
		return false;
	}
}
void DX9_GpuApi::OnLostDevice()
{
	//for(DX9_ShaderBank::iterator i = shaderBank.begin(); i != shaderBank.end(); ++i)
	//{
	//	i->second->shaderObject->OnLostDevice();
	//}
	if(spriteInterface) spriteInterface->OnLostDevice();
	for(std::list<IGpuDeviceListener*>::iterator i = deviceListeners.begin();
		i != deviceListeners.end(); ++i)
	{
		(*i)->OnLostDevice(this);
	}
}
void DX9_GpuApi::OnResetDevice()
{
	//for(DX9_ShaderBank::iterator i = shaderBank.begin(); i != shaderBank.end(); ++i)
	//{
	//	i->second->shaderObject->OnResetDevice();
	//}
	if(spriteInterface) spriteInterface->OnResetDevice();
	for(std::list<IGpuDeviceListener*>::iterator i = deviceListeners.begin();
		i != deviceListeners.end(); ++i)
	{
		(*i)->OnResetDevice(this);
	}
}

struct DX9_GpuMeshCopy : public GpuMeshCopy
{
	DX9_GpuMesh * target;
	unsigned vbOffset;
	unsigned vbSize;
	unsigned ibOffset;
	unsigned ibSize;

	DX9_GpuMeshCopy(LocalMesh * localMesh, DX9_GpuMesh * target) :
		GpuMeshCopy(localMesh),
		target(target), 
		vbOffset(0),
		vbSize(localMesh->vertexBuffer->GetLength() * localMesh->vertexBuffer->GetElementSize()),
		ibOffset(0),
		ibSize(localMesh->GetNumIndices() * sizeof(unsigned)) {}
	~DX9_GpuMeshCopy() { if(localMesh) delete localMesh; }

	virtual void Step() override
	{
		void* data = 0;
		unsigned copySize = 1024;

		if(vbOffset < vbSize)
		{
			char * vbInput = (char*) localMesh->vertexBuffer->GetData();

			if(vbSize - vbOffset < 1024)
			{
				copySize = vbSize - vbOffset;
			}

			target->vertexBuffer->Lock(vbOffset, copySize, &data, 0);
			memcpy(data, &(vbInput[vbOffset]), copySize);
			target->vertexBuffer->Unlock();

			vbOffset += copySize;
		}
		else if(ibOffset < ibSize)
		{
			char * ibInput = (char*) localMesh->indexBuffer;

			if(ibSize - ibOffset < 1024)
			{
				copySize = ibSize - ibOffset;
			}

			target->indexBuffer->Lock(ibOffset, copySize, &data, 0);
			memcpy(data, &(ibInput[ibOffset]), copySize);
			target->indexBuffer->Unlock();

			ibOffset += copySize;
		}
	}

	virtual float GetProgress() override
	{
		return float(vbOffset + ibOffset) / float(vbSize + ibSize);
	}
	virtual bool IsFinished() override
	{
		return vbOffset >= vbSize && ibOffset >= ibSize;
	}
	virtual GpuMesh * GetMesh() override
	{
		return target;
	}
};

GpuMesh * DX9_GpuApi::CreateGpuMesh(unsigned numVertices, void * vertexData, VertexType type, bool dynamic)
{
	DX9_GpuMesh * createdMesh = new DX9_GpuMesh(this);

	createdMesh->dynamic = dynamic;
	createdMesh->vertexSize = VertApi::GetVertexSize(type);
	createdMesh->vertexType = type;
	createdMesh->numVertices = numVertices;
	
	unsigned vertexDataSize = numVertices * createdMesh->vertexSize;

	if(dynamic) 
	{
		createdMesh->localVertexData = new char[vertexDataSize];
		memcpy(createdMesh->localVertexData, vertexData, vertexDataSize);
		deviceListeners.push_back(createdMesh);
	}

	CreateVertexBuffer(&createdMesh->vertexBuffer, vertexDataSize, dynamic);
	FillVertexBuffer(&createdMesh->vertexBuffer, vertexData, vertexDataSize, dynamic);

	return createdMesh;
}
GpuMesh * DX9_GpuApi::CreateGpuMesh(unsigned numVertices, void * vertexData,
	unsigned numTriangles, unsigned * indexData, VertexType type, bool dynamic)
{
	DX9_GpuMesh * createdMesh = static_cast<DX9_GpuMesh*>(CreateGpuMesh(numVertices, vertexData, type, dynamic));

	createdMesh->numTriangles = numTriangles;
	
	unsigned indexDataSize = sizeof(unsigned) * numTriangles * 3;

	if(dynamic)
	{
		createdMesh->localIndexData = new unsigned[numTriangles * 3];
		memcpy(createdMesh->localIndexData, indexData, indexDataSize);
	}

	CreateIndexBuffer(&createdMesh->indexBuffer, indexDataSize, dynamic);
	FillIndexBuffer(&createdMesh->indexBuffer, indexData, indexDataSize, dynamic);

	return createdMesh;
}
GpuInstanceBuffer * DX9_GpuApi::CreateInstanceBuffer(unsigned numInstances, void * instanceData, InstanceType type)
{
	DX9_GpuInstanceBuffer * createdBuffer = new DX9_GpuInstanceBuffer(this);

	createdBuffer->instanceSize = VertApi::GetInstanceSize(type);
	createdBuffer->type = type;
	createdBuffer->numInstances = numInstances;
	createdBuffer->instanceCapacity = numInstances;

	unsigned instanceDataSize = createdBuffer->instanceSize * numInstances;

	createdBuffer->localBuffer = new char[instanceDataSize];
	memcpy(createdBuffer->localBuffer, instanceData, instanceDataSize);

	CreateVertexBuffer(&createdBuffer->buffer, instanceDataSize, true);
	FillVertexBuffer(&createdBuffer->buffer, instanceData, instanceDataSize, true);

	deviceListeners.push_back(createdBuffer);

	return createdBuffer;
}
GpuMeshCopy * DX9_GpuApi::CreateLargeGpuMesh(LocalMesh * localMesh)
{
	if(!localMesh) return 0;

	DX9_GpuMesh * createdMesh = new DX9_GpuMesh(this);
	createdMesh->dynamic = false;
	createdMesh->vertexSize = localMesh->vertexBuffer->GetElementSize();
	createdMesh->vertexType = localMesh->vertexBuffer->GetVertexType();
	createdMesh->numVertices = localMesh->vertexBuffer->GetLength();
	createdMesh->numTriangles = localMesh->numTriangles;

	unsigned vertexDataSize = createdMesh->vertexSize * createdMesh->numVertices;
	unsigned indexDataSize = sizeof(unsigned) * createdMesh->numTriangles * 3;

	CreateVertexBuffer(&createdMesh->vertexBuffer, vertexDataSize, false);
	CreateIndexBuffer(&createdMesh->indexBuffer, indexDataSize, false);

	DX9_GpuMeshCopy * copyTask = new DX9_GpuMeshCopy(localMesh, createdMesh);
	steppables->Add(copyTask);
	return copyTask;
}

void DX9_GpuApi::UpdateDynamicMesh(GpuMesh * mesh, IVertexBuffer * buffer)
{
	DX9_GpuMesh * dx9mesh = static_cast<DX9_GpuMesh*>(mesh);
	if(!dx9mesh->IsDynamic())
	{
		OutputDebugString(L"Attempted to update a mesh that was not dynamic\n");
		return;
	}
	if(buffer->GetLength() > dx9mesh->numVertices)
	{
		OutputDebugString(L"Attempted to overflow a smaller vertex buffer\n");
		return;
		// TODO: Recreate vertex buffer
	}

	unsigned vertexDataSize = buffer->GetLength() * buffer->GetElementSize();
	memcpy(dx9mesh->localVertexData, buffer->GetData(), vertexDataSize);

	FillVertexBuffer(&dx9mesh->vertexBuffer, dx9mesh->localVertexData, vertexDataSize, true);
}
void DX9_GpuApi::UpdateDynamicMesh(GpuMesh * mesh, IVertexBuffer * buffer, unsigned numTriangles, unsigned * indexData)
{
	DX9_GpuMesh * dx9mesh = static_cast<DX9_GpuMesh*>(mesh);
	if(!dx9mesh->IsDynamic())
	{
		OutputDebugString(L"Attempted to update a mesh that was not dynamic\n");
		return;
	}
	if(!dx9mesh->IsIndexed())
	{
		OutputDebugString(L"Attempted to update the indices of a non-indexed mesh\n");
		return;
	}
	if(buffer->GetLength() > dx9mesh->numVertices)
	{
		OutputDebugString(L"Attempted to overflow a smaller vertex buffer\n");
		return;
		// TODO: Recreate vertex buffer
	}
	if(numTriangles > dx9mesh->numTriangles)
	{
		OutputDebugString(L"Attempted to overflow a smaller index buffer\n");
		return;
		// TODO: Recreate index buffer
	}

	unsigned vertexDataSize = buffer->GetLength() * buffer->GetElementSize();
	memcpy(dx9mesh->localVertexData, buffer->GetData(), vertexDataSize);
	unsigned indexDataSize = sizeof(unsigned) * numTriangles * 3;
	memcpy(dx9mesh->localIndexData, indexData, indexDataSize);

	FillVertexBuffer(&dx9mesh->vertexBuffer, dx9mesh->localVertexData, vertexDataSize, true);
	FillIndexBuffer(&dx9mesh->indexBuffer, dx9mesh->localIndexData, indexDataSize, true);
}
void DX9_GpuApi::UpdateInstanceBuffer(GpuInstanceBuffer * instanceBuffer, unsigned numInstances, void * instanceData)
{
	DX9_GpuInstanceBuffer * dx9instanceBuffer = static_cast<DX9_GpuInstanceBuffer*>(instanceBuffer);

	if(numInstances > dx9instanceBuffer->instanceCapacity)
	{
		OutputDebugString(L"Attempted to overflow a smaller instance buffer\n");
		return;
	}

	dx9instanceBuffer->numInstances = numInstances;

	unsigned instanceDataSize = dx9instanceBuffer->instanceSize * numInstances;
	memcpy(dx9instanceBuffer->localBuffer, instanceData, instanceDataSize);

	FillVertexBuffer(&dx9instanceBuffer->buffer, instanceData, instanceDataSize, true);
}

GpuDrawSurface * DX9_GpuApi::CreateDrawSurface(unsigned width, unsigned height)
{
	return new DX9_GpuTextureSurface(this,direct3Ddevice,false,width,height);
}

GpuDrawSurface * DX9_GpuApi::CreateFullscreenDrawSurface()
{
	return new DX9_GpuTextureSurface(this,direct3Ddevice,true,
		presentParameters.BackBufferWidth,presentParameters.BackBufferHeight);
}

float DX9_GpuApi::MeasureGpuText(GpuFont * font, const wchar_t * text)
{
	RECT rect = {0};
	DX9_GpuFont * dxfont = static_cast<DX9_GpuFont*>(font);
	dxfont->fontObject->DrawTextW(0, text, -1, &rect, DT_CALCRECT, 
		floatToD3DColor(1.0,1.0,1.0,1.0));

	// Perhaps this should respond to dxfont->pixelSpace()?
	return float(rect.right);
}

DX9_GpuFont::DX9_GpuFont(DX9_GpuApi * gpu, ID3DXFont * object) : 
GpuFont(), dx9gpu(gpu), fontObject(object), size(1.0f)
{
	if(fontObject) 
	{
		dx9gpu->deviceListeners.push_back(this);
	}
}

DX9_GpuFont::~DX9_GpuFont()
{
	if(fontObject) 
	{
		dx9gpu->deviceListeners.remove(this);
		fontObject->Release(); 
	}
}

DX9_GpuMesh * DX9_GpuApi::D3DXToGpuMesh(ID3DXMesh* d3dxmesh)
{
	IDirect3DVertexBuffer9 * vertexBuffer;
	IDirect3DIndexBuffer9 * indexBuffer;

	d3dxmesh->GetVertexBuffer(&vertexBuffer);
	d3dxmesh->GetIndexBuffer(&indexBuffer);

	DX9_GpuMesh* createdMesh = new DX9_GpuMesh(this);

	createdMesh->vertexBuffer = vertexBuffer;
	createdMesh->indexBuffer = indexBuffer;

	createdMesh->numVertices = d3dxmesh->GetNumVertices();
	createdMesh->numTriangles = d3dxmesh->GetNumFaces();

	//createdMesh->vertexDeclaration = vertexDeclarations[VertexType_PosNor];
	createdMesh->vertexSize = sizeof(Vertex_PosNor);
	createdMesh->vertexType = VertexType_PosNor;
	
	return createdMesh;
}

void DX9_GpuApi::CreateVertexBuffer(IDirect3DVertexBuffer9 ** output, unsigned dataSize, bool dynamic)
{
	direct3Ddevice->CreateVertexBuffer(
		dataSize,
		dynamic ? D3DUSAGE_DYNAMIC : D3DUSAGE_WRITEONLY,
		0,
		dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		output,
		0);
}

void DX9_GpuApi::FillVertexBuffer(IDirect3DVertexBuffer9 ** target, void * input, unsigned dataSize, bool dynamic)
{
	DWORD flags = dynamic ? D3DLOCK_DISCARD : 0;

	void* data = 0;
	(*target)->Lock(0, dataSize, &data, flags);
	memcpy(data, input, dataSize);
	(*target)->Unlock();
}

void DX9_GpuApi::CreateIndexBuffer(IDirect3DIndexBuffer9 ** output, unsigned dataSize, bool dynamic)
{
	direct3Ddevice->CreateIndexBuffer(
		dataSize,
		dynamic ? D3DUSAGE_DYNAMIC : D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX32,
		dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		output,
		0);
}

void DX9_GpuApi::FillIndexBuffer(IDirect3DIndexBuffer9 ** target, void * input, unsigned dataSize, bool dynamic)
{
	DWORD flags = dynamic ? D3DLOCK_DISCARD : 0;

	void* data = 0;
	(*target)->Lock(0, dataSize, &data, flags);
	memcpy(data, input, dataSize);
	(*target)->Unlock();
}

void DX9_GpuApi::RecreateMesh(DX9_GpuMesh * mesh)
{
	unsigned vertexDataSize = mesh->numVertices * mesh->vertexSize;
	CreateVertexBuffer(&mesh->vertexBuffer, vertexDataSize, mesh->dynamic);
	FillVertexBuffer(&mesh->vertexBuffer, mesh->localVertexData, vertexDataSize, mesh->dynamic);
	if(mesh->IsIndexed())
	{
		unsigned indexDataSize = sizeof(unsigned) * mesh->numTriangles * 3;
		CreateIndexBuffer(&mesh->indexBuffer, indexDataSize, mesh->dynamic);
		FillIndexBuffer(&mesh->indexBuffer, mesh->localIndexData, indexDataSize, mesh->dynamic);
	}
}

void DX9_GpuApi::RecreateInstanceBuffer(DX9_GpuInstanceBuffer * instanceBuffer)
{
	CreateVertexBuffer(&instanceBuffer->buffer, instanceBuffer->instanceCapacity * instanceBuffer->instanceSize, true);
	FillVertexBuffer(&instanceBuffer->buffer, instanceBuffer->localBuffer, instanceBuffer->instanceCapacity * instanceBuffer->instanceSize, true);
}

DX9_GpuMesh::~DX9_GpuMesh()
{
	if(vertexBuffer) vertexBuffer->Release();
	if(indexBuffer) indexBuffer->Release(); indexBuffer = 0;
	if(localVertexData) delete[] localVertexData; localVertexData = 0;
	if(localIndexData) delete[] localIndexData; localIndexData = 0;
	if(dynamic)
	{
		// FIXME - if(!dx9gpu) ERROR!!
		dx9gpu->RemoveDeviceListener(this);
	}
}

void DX9_GpuInstanceBuffer::OnLostDevice(GpuApi * gpu)
{
	if(buffer) buffer->Release();
}

void DX9_GpuInstanceBuffer::OnResetDevice(GpuApi * gpu)
{
	dx9gpu->RecreateInstanceBuffer(this);
}

DX9_GpuInstanceBuffer::~DX9_GpuInstanceBuffer()
{
	if(buffer) buffer->Release(); buffer = 0;
	if(localBuffer) delete[] localBuffer; localBuffer = 0;
	dx9gpu->RemoveDeviceListener(this);
}

#endif
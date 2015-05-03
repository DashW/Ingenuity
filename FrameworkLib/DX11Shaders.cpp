#ifdef USE_DX11_GPUAPI

#include "DX11Shaders.h"
#include "DX11Api.h"
#include "DX11Surfaces.h"
#include "tinyxml2.h"

#include <sstream>

using namespace DirectX;

namespace Ingenuity {

void DX11::Shader::ApplyTextureParameter(ID3D11DeviceContext * direct3Dcontext, ShaderStage stage, unsigned registerIndex, Gpu::ShaderParam * param)
{
	if(!param || !param->tvalue) return;

	ID3D11ShaderResourceView * resource = 0;

	switch(param->type)
	{
	case Gpu::ShaderParam::TypeTexture:
		resource = static_cast<Texture*>(param->tvalue)->shaderView;
		break;
	case Gpu::ShaderParam::TypeCubeTexture:
		resource = static_cast<CubeMap*>(param->cvalue)->shaderView;
		break;
	case Gpu::ShaderParam::TypeVolumeTexture:
		resource = static_cast<VolumeTexture*>(param->vvalue)->shaderView;
		break;
	case Gpu::ShaderParam::TypeDrawSurface:
		resource = static_cast<Texture*>(param->svalue->GetTexture())->shaderView;
		break;
	}

	switch(stage)
	{
	case Compute:
		direct3Dcontext->CSSetShaderResources(registerIndex, 1, &resource);
		break;
	case Vertex:
		direct3Dcontext->VSSetShaderResources(registerIndex, 1, &resource);
		break;
	case Geometry:
		direct3Dcontext->GSSetShaderResources(registerIndex, 1, &resource);
		break;
	case Pixel:
		direct3Dcontext->PSSetShaderResources(registerIndex, 1, &resource);
		break;
	}
}

void DX11::Shader::UpdateConstantBuffer(ID3D11DeviceContext * context, std::vector<float> & constants, ID3D11Buffer ** buffer)
{
	while((constants.size() % 4) != 0)
	{
		constants.push_back(0.0f);
	}

	if(*buffer)
	{
		context->UpdateSubresource(*buffer, 0, 0, constants.data(), 0, 0);
	}
	else
	{
		ID3D11Device * device = 0;
		context->GetDevice(&device);

		if(device)
		{
			D3D11_SUBRESOURCE_DATA data = { 0 };
			data.pSysMem = constants.data();

			device->CreateBuffer(
				&CD3D11_BUFFER_DESC(sizeof(float) * constants.size(), D3D11_BIND_CONSTANT_BUFFER),
				&data,
				buffer);

			device->Release();
		}
	}
}

DX11::ModelShader::Technique::Technique() 
	: inputLayout(0)
	, vertexObject(0)
	, geometryObject(0)
	, pixelObject(0)
{
	ZeroMemory(vertexParamConstBuffers, NUM_PARAM_BUFFERS * sizeof(void*));
	ZeroMemory(geometryParamConstBuffers, NUM_PARAM_BUFFERS * sizeof(void*));
	ZeroMemory(pixelParamConstBuffers, NUM_PARAM_BUFFERS * sizeof(void*));
}
DX11::ModelShader::Technique::~Technique()
{
	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		if(vertexParamConstBuffers[i]) vertexParamConstBuffers[i]->Release();
		if(geometryParamConstBuffers[i]) geometryParamConstBuffers[i]->Release();
		if(pixelParamConstBuffers[i]) pixelParamConstBuffers[i]->Release();
	}

	if(inputLayout) inputLayout->Release();
	if(vertexObject) vertexObject->Release();
	if(geometryObject) geometryObject->Release();
	if(pixelObject) pixelObject->Release();
}
bool DX11::ModelShader::Technique::SetExtraParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect)
{
	unsigned numParams = effect->shader->paramSpecs.size();
	if(paramMappings.size() > numParams)
	{
		OutputDebugString(L"Too many parameter mappings for the specified parameters!\n");
		return false;
	}

	for(unsigned i = 0; i < paramMappings.size(); ++i)
	{
		ParamMapping & mapping = paramMappings[i];

		Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[mapping.paramIndex];
		Gpu::ShaderParam * param = effect->params[mapping.paramIndex];

		if(!param) return false;
		if(param->type != spec.type) return false;

		std::vector<float> * stageParamConstData = 0;
		switch(mapping.shader)
		{
		case ShaderStage::Vertex: stageParamConstData = vertexParamConstData; break;
		case ShaderStage::Geometry: stageParamConstData = geometryParamConstData; break;
		case ShaderStage::Pixel: stageParamConstData = pixelParamConstData; break;
		default:
			OutputDebugString(L"Non vertex/geometry/pixel shader states not yet implemented!\n");
			return false;
		}

		switch(spec.type)
		{
		case Gpu::ShaderParam::TypeFloat:
		{
			if(mapping.registerIndex < NUM_STANDARD_BUFFERS
				|| mapping.registerIndex >= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT)
			{
				OutputDebugString(L"Invalid shader parameter mapping register index!\n");
				return false;
			}
			std::vector<float> & constants = stageParamConstData[mapping.registerIndex - NUM_STANDARD_BUFFERS];
			while(constants.size() <= mapping.bufferOffset) constants.push_back(0.0f);
			constants[mapping.bufferOffset] = param->fvalue;
			break;
		}
		case Gpu::ShaderParam::TypeFloatArray:
		{
			if(mapping.registerIndex < NUM_STANDARD_BUFFERS
				|| mapping.registerIndex >= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT)
			{
				OutputDebugString(L"Invalid shader parameter mapping register index!\n");
				return false;
			}
			if(param->avalue == 0)
			{
				OutputDebugString(L"Unset shader parameter!\n");
				return false;
			}
			std::vector<float> & constants = stageParamConstData[mapping.registerIndex - NUM_STANDARD_BUFFERS];
			constants.resize(mapping.bufferOffset + param->avalue->numFloats + 1);
			memcpy(&(constants[mapping.bufferOffset]), param->avalue->floats, param->avalue->numFloats * sizeof(float));
			break;
		}
		case Gpu::ShaderParam::TypeParamBuffer:
		{
			DX11::ParamBuffer * paramBuffer = static_cast<DX11::ParamBuffer*>(param->bvalue);
			if(mapping.writeable)
			{
				OutputDebugString(L"Cannot assign a writeable param buffer to a non-compute shader!\n");
				return false;
			}
			if(param->bvalue == 0)
			{
				OutputDebugString(L"Unset shader parameter!\n");
				return false;
			}
			switch(mapping.shader)
			{
			case ShaderStage::Vertex:
				direct3Dcontext->VSSetShaderResources(mapping.registerIndex, 1, &paramBuffer->srv);
				break;
			case ShaderStage::Geometry:
				direct3Dcontext->GSSetShaderResources(mapping.registerIndex, 1, &paramBuffer->srv);
				break;
			case ShaderStage::Pixel:
				direct3Dcontext->PSSetShaderResources(mapping.registerIndex, 1, &paramBuffer->srv);
				break;
			}
			break;
		}
		default:
			ApplyTextureParameter(direct3Dcontext, mapping.shader, mapping.registerIndex, param);
			break;
		}
	}

	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		// Not sure if faster to SetConstantBuffers specifically for those that have data, or all in one go regardless of data

		if(vertexParamConstData[i].size() > 0)
		{
			UpdateConstantBuffer(direct3Dcontext, vertexParamConstData[i], &vertexParamConstBuffers[i]);
			direct3Dcontext->VSSetConstantBuffers(i + NUM_STANDARD_BUFFERS, 1, &vertexParamConstBuffers[i]);
		}

		if(geometryParamConstData[i].size() > 0)
		{
			UpdateConstantBuffer(direct3Dcontext, geometryParamConstData[i], &geometryParamConstBuffers[i]);
			direct3Dcontext->GSSetConstantBuffers(i + NUM_STANDARD_BUFFERS, 1, &geometryParamConstBuffers[i]);
		}

		if(pixelParamConstData[i].size() > 0)
		{
			UpdateConstantBuffer(direct3Dcontext, pixelParamConstData[i], &pixelParamConstBuffers[i]);
			direct3Dcontext->PSSetConstantBuffers(i + NUM_STANDARD_BUFFERS, 1, &pixelParamConstBuffers[i]);
		}
	}

	return true;
}

DX11::ModelShader::ModelShader(ID3D11Device * device) 
	: DX11::Shader(device, Type::Model)
	, vertexConstBuffer(0)
	, pixelConstBuffer(0)
	, indirectArgsBuffer(0)
	, currentTechnique(0)
	, indirectPrimitive(PrimitiveUnknown)
{
	device->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(VertexConstants), D3D11_BIND_CONSTANT_BUFFER),
		0,
		&vertexConstBuffer);
	device->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(PixelConstants), D3D11_BIND_CONSTANT_BUFFER),
		0,
		&pixelConstBuffer);
	device->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(LightConstants)* MAX_LIGHTS, D3D11_BIND_CONSTANT_BUFFER),
		0,
		&lightParamsBuffer);
}
DX11::ModelShader::~ModelShader()
{
	if(vertexConstBuffer) vertexConstBuffer->Release();
	if(pixelConstBuffer) pixelConstBuffer->Release();
	if(lightParamsBuffer) lightParamsBuffer->Release();
	if(indirectArgsBuffer) indirectArgsBuffer->Release();
}

bool DX11::ModelShader::SetTechnique(ID3D11DeviceContext * direct3Dcontext, VertexType vType, InstanceType iType)
{
	unsigned key = VertApi::GetTechniqueKey(vType, iType);

	std::map<unsigned, Technique>::iterator it = vertexTechniques.find(key);
	if(it == vertexTechniques.end())
	{
		OutputDebugString(L"Shader does not have technique for vertex/instance type!\n");
		return false;
	}

	currentTechnique = &(it->second);

	direct3Dcontext->IASetInputLayout(currentTechnique->inputLayout);
	direct3Dcontext->VSSetShader(currentTechnique->vertexObject, 0, 0);
	direct3Dcontext->GSSetShader(currentTechnique->geometryObject, 0, 0);
	direct3Dcontext->PSSetShader(currentTechnique->pixelObject, 0, 0);

	return true;
}

bool DX11::ModelShader::SetIndirectTechnique(ID3D11DeviceContext * direct3Dcontext)
{
	if(!indirectTechnique.vertexObject) return false;

	currentTechnique = &indirectTechnique;

	switch(indirectPrimitive)
	{
	case IndirectPrimitive::PrimitivePoint:
		direct3Dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		break;
	case IndirectPrimitive::PrimitiveLine:
		direct3Dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		break;
	case IndirectPrimitive::PrimitiveTriangle:
		direct3Dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	}

	direct3Dcontext->VSSetShader(currentTechnique->vertexObject, 0, 0);
	direct3Dcontext->GSSetShader(currentTechnique->geometryObject, 0, 0);
	direct3Dcontext->PSSetShader(currentTechnique->pixelObject, 0, 0);

	return true;
}

bool DX11::ModelShader::SetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights, unsigned numLights, float aspect, Gpu::Effect * effect)
{
	if(!model || !camera) return false;
	if(camera->position == camera->target)
	{
		OutputDebugString(L"Cannot construct view matrix!");
		return false;
	}

	// MATRIX OPS - SIGNIFICANT COST - ~9us //

	glm::mat4 modelMatrix = model->GetMatrix();
	XMMATRIX world((float*)&modelMatrix);

	XMStoreFloat4x4(&vertexConstData.world, world);

	glm::mat4 camMatrix = camera->GetProjMatrix(aspect) * camera->GetViewMatrix();
	XMMATRIX viewProj((float*)&camMatrix);

	XMStoreFloat4x4(&vertexConstData.viewProjection, viewProj);

	XMMATRIX wit = XMMatrixInverse(0, XMMatrixTranspose(world));
	XMStoreFloat4x4(&vertexConstData.worldInverseTranspose, wit);

	vertexConstData.materialColor = XMFLOAT4(model->color.r, model->color.g, model->color.b, model->color.a);

	ID3D11ShaderResourceView * nullResource = 0;

	if(model->texture)
	{
		DX11::Texture* texture = static_cast<DX11::Texture*>(model->texture);
		direct3Dcontext->PSSetShaderResources(0, 1, &texture->shaderView);
	}
	else
	{
		direct3Dcontext->PSSetShaderResources(0, 1, &nullResource);
	}
	
	// BEGIN LIGHTING - VARIABLE COST //

	if(model->cubeMap)
	{
		DX11::CubeMap * cubeMap = static_cast<DX11::CubeMap*>(model->cubeMap);
		direct3Dcontext->PSSetShaderResources(1, 1, &cubeMap->shaderView);
		pixelConstData.cubeMapAlpha = 1.0f;
	}
	else
	{
		direct3Dcontext->PSSetShaderResources(1, 1, &nullResource);
		pixelConstData.cubeMapAlpha = 0.0f;
	}
	if(model->normalMap)
	{
		DX11::Texture * normalMap = static_cast<DX11::Texture*>(model->normalMap);
		direct3Dcontext->PSSetShaderResources(2, 1, &normalMap->shaderView);
	}
	else
	{
		direct3Dcontext->PSSetShaderResources(2, 1, &nullResource);
	}

	pixelConstData.numLights = numLights;
	if(numLights > 0)
	{
		pixelConstData.ambient = 0.0f;
		pixelConstData.cameraPosition = XMFLOAT3(camera->position.x, camera->position.y, camera->position.z);
		pixelConstData.specularPower = model->specPower;
		pixelConstData.specularFactor = model->specFactor;
		pixelConstData.diffuseFactor = model->diffuseFactor;

		for(unsigned i = 0; i < numLights && i < MAX_LIGHTS; ++i)
		{
			// Hmm, really the specular highlight should be calculated
			// from the light's size, position and intensity.
			// Do a little more investigation into Physically Based Lighting

			lightConstData.positionSpecs[i].specPower = 12.0f;

			Gpu::LightType lightType = lights[i]->GetType();
			if(lightType == Gpu::LightType_Directional)
			{
				Gpu::DirectionalLight * dirLight = static_cast<Gpu::DirectionalLight*>(lights[i]);
				XMVECTOR lightDirVec = XMLoadFloat3(&XMFLOAT3(dirLight->direction.x, dirLight->direction.y, dirLight->direction.z));
				XMStoreFloat3(&(lightConstData.positionSpecs[i].position), XMVectorScale(lightDirVec, 10.0e+10f));
				lightConstData.colorAttenuations[i].color = XMFLOAT3(dirLight->color.r, dirLight->color.g, dirLight->color.b);
				lightConstData.colorAttenuations[i].attenuation = 0.0f;
				lightConstData.spotDirPowers[i].direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
				lightConstData.spotDirPowers[i].spotPower = 0.0f;
			}
			if(lightType == Gpu::LightType_Point)
			{
				Gpu::PointLight * pointLight = static_cast<Gpu::PointLight*>(lights[i]);
				lightConstData.positionSpecs[i].position = XMFLOAT3(pointLight->position.x, pointLight->position.y, pointLight->position.z);
				lightConstData.colorAttenuations[i].color = XMFLOAT3(pointLight->color.r, pointLight->color.g, pointLight->color.b);
				lightConstData.colorAttenuations[i].attenuation = pointLight->atten;
				lightConstData.spotDirPowers[i].direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
				lightConstData.spotDirPowers[i].spotPower = 0.0f;
			}
			if(lightType == Gpu::LightType_Spot)
			{
				Gpu::SpotLight * spotLight = static_cast<Gpu::SpotLight*>(lights[i]);
				lightConstData.positionSpecs[i].position = XMFLOAT3(spotLight->position.x, spotLight->position.y, spotLight->position.z);
				lightConstData.colorAttenuations[i].color = XMFLOAT3(spotLight->color.r, spotLight->color.g, spotLight->color.b);
				lightConstData.colorAttenuations[i].attenuation = spotLight->atten;
				XMStoreFloat3(&lightConstData.spotDirPowers[i].direction, XMVector3Normalize(XMLoadFloat3(
					&XMFLOAT3(spotLight->direction.x, spotLight->direction.y, spotLight->direction.z))));
				lightConstData.spotDirPowers[i].spotPower = spotLight->power;

				if(i == 0)
				{
					float lightFOV = XM_PI * 0.15f;

					glm::mat4 lightGlmMatrix = spotLight->GetMatrix(lightFOV, glm::vec3(0.0f, 1.0f, 0.0f));
					XMMATRIX lightXMMatrix((float*)&lightGlmMatrix);

					//XMMATRIX lightView = XMMatrixLookToLH(
					//	XMLoadFloat3(&lightConstData.positionSpecs[i].position),
					//	XMLoadFloat3(&lightConstData.spotDirPowers[i].direction),
					//	XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f)));

					//XMMATRIX lightLens = XMMatrixPerspectiveFovLH(lightFOV, 1.0f, 1.0f, 200.0f);

					XMStoreFloat4x4(&vertexConstData.lightViewProjection, lightXMMatrix);
				}
			}
		}

		direct3Dcontext->UpdateSubresource(lightParamsBuffer, 0, 0, &lightConstData, 0, 0);
		direct3Dcontext->PSSetConstantBuffers(1, 1, &lightParamsBuffer);
	}
	else
	{
		pixelConstData.lightPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
		pixelConstData.lightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		pixelConstData.cameraPosition = XMFLOAT3(camera->position.x, camera->position.y, camera->position.z);
		pixelConstData.spotPower = 0.0f;
		pixelConstData.specularPower = FLT_MAX;
		pixelConstData.spotDirection = XMFLOAT3(0.0f, 0.0f, 0.0f);
		pixelConstData.ambient = 1.0f;
	}

	// END LIGHTING //

	// UPDATING CONSTANT BUFFERS - SIGNIFICANT COST - 17us //

	direct3Dcontext->UpdateSubresource(pixelConstBuffer, 0, 0, &pixelConstData, 0, 0);
	direct3Dcontext->PSSetConstantBuffers(0, 1, &pixelConstBuffer);

	direct3Dcontext->UpdateSubresource(vertexConstBuffer, 0, 0, &vertexConstData, 0, 0);
	direct3Dcontext->VSSetConstantBuffers(0, 1, &vertexConstBuffer);

	// SETTING EXTRA PARAMS - VARIABLE COST //

	if(effect)
	{
		return currentTechnique->SetExtraParameters(direct3Dcontext, effect);
	}
	else
	{
		return true;
	}
}

void DX11::ModelShader::UnsetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect)
{
	if(!currentTechnique) return;

	for(unsigned i = 0; i < currentTechnique->paramMappings.size(); ++i)
	{
		ParamMapping & mapping = currentTechnique->paramMappings[i];

		Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[mapping.paramIndex];
		Gpu::ShaderParam * param = effect->params[mapping.paramIndex];

		switch(spec.type)
		{
		case Gpu::ShaderParam::TypeParamBuffer:
		{
			ID3D11ShaderResourceView * nullSrv = 0;
			switch(mapping.shader)
			{
			case ShaderStage::Vertex:
				direct3Dcontext->VSSetShaderResources(mapping.registerIndex, 1, &nullSrv);
				break;
			case ShaderStage::Geometry:
				direct3Dcontext->GSSetShaderResources(mapping.registerIndex, 1, &nullSrv);
				break;
			case ShaderStage::Pixel:
				direct3Dcontext->PSSetShaderResources(mapping.registerIndex, 1, &nullSrv);
				break;
			}
			break;
		}
		}
	}
}

void DX11::ModelShader::CreateIndirectBuffer(ID3D11Device * device)
{
	struct DrawInstancedIndirectArgs {
		UINT VertexCountPerInstance;
		UINT InstanceCount;
		UINT StartVertexLocation;
		UINT StartInstanceLocation;

		DrawInstancedIndirectArgs()
			: VertexCountPerInstance(1)
			, InstanceCount(1)
			, StartVertexLocation(0)
			, StartInstanceLocation(0)
		{}
	};

	DrawInstancedIndirectArgs args;

	D3D11_BUFFER_DESC desc = { 0 };
	desc.ByteWidth = sizeof(DrawInstancedIndirectArgs);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

	D3D11_SUBRESOURCE_DATA argsData = { 0 };
	argsData.pSysMem = &args;

	HRESULT hr = device->CreateBuffer(&desc, &argsData, &indirectArgsBuffer);
}

ID3D11Buffer * DX11::ModelShader::GetIndirectBuffer()
{
	return indirectArgsBuffer;
}

ID3D11VertexShader * DX11::TextureShader::vertexObject = 0;

DX11::TextureShader::TextureShader(ID3D11Device * device) :
	DX11::Shader(device, Type::Texture), standardConstBuffer(0), pixelObject(0)
{
	device->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(StandardConstants), D3D11_BIND_CONSTANT_BUFFER),
		0,
		&standardConstBuffer);

	ZeroMemory(paramConstBuffers, NUM_PARAM_BUFFERS * sizeof(void*));
}
DX11::TextureShader::~TextureShader()
{
	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		if(paramConstBuffers[i]) paramConstBuffers[i]->Release();
	}

	if(standardConstBuffer) standardConstBuffer->Release();
	if(pixelObject) pixelObject->Release();
}

bool DX11::TextureShader::SetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Texture * texture, Gpu::Effect * effect)
{
	if(!texture) return false;
	DX11::Texture * dx11tex = static_cast<DX11::Texture*>(texture);

	direct3Dcontext->PSSetShaderResources(0, 1, &dx11tex->shaderView);

	standardConstData.texWidth = float(texture->GetWidth());
	standardConstData.texHeight = float(texture->GetHeight());

	direct3Dcontext->UpdateSubresource(standardConstBuffer, 0, 0, &standardConstData, 0, 0);
	direct3Dcontext->PSSetConstantBuffers(0, 1, &standardConstBuffer);

	direct3Dcontext->PSSetShader(pixelObject, 0, 0);

	if(effect)
	{
		return SetExtraParameters(direct3Dcontext, effect);
	}
	else
	{
		return true;
	}
}

bool DX11::TextureShader::SetExtraParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect)
{
	unsigned numParams = effect->shader->paramSpecs.size();
	if(paramMappings.size() > numParams)
	{
		OutputDebugString(L"Too many parameter mappings for the specified parameters!\n");
		return false;
	}

	for(unsigned i = 0; i < paramMappings.size(); ++i)
	{
		ParamMapping & mapping = paramMappings[i];

		Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[mapping.paramIndex];
		Gpu::ShaderParam * param = effect->params[mapping.paramIndex];

		if(!param) return false;
		if(param->type != spec.type)
		{
			OutputDebugString(L"Parameter type does not match specified parameter type!\n");
			return false;
		}

		switch(spec.type)
		{
		case Gpu::ShaderParam::TypeFloat:
		{
			std::vector<float> & constants = paramConstData[mapping.registerIndex - 1];
			while(constants.size() <= mapping.bufferOffset)
			{
				constants.push_back(0.0f);
			}
			constants[mapping.bufferOffset] = param->fvalue;
			break;
		}
		case Gpu::ShaderParam::TypeFloatArray:
		{
			if(param->avalue == 0)
			{
				OutputDebugString(L"Unset shader parameter!\n");
				return false;
			}
			std::vector<float> & constants = paramConstData[mapping.registerIndex - 1];
			const unsigned minBufferSize = mapping.bufferOffset + param->avalue->numFloats;
			if(constants.size() < minBufferSize) constants.resize(minBufferSize);
			memcpy(&(constants[mapping.bufferOffset]), param->avalue->floats, param->avalue->numFloats * sizeof(float));
			break;
		}
		default:
			ApplyTextureParameter(direct3Dcontext, mapping.shader, mapping.registerIndex, param);
			break;
		}
	}

	if(numParams > 0)
	{
		for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
		{
			if(paramConstData[i].size() > 0)
			{
				UpdateConstantBuffer(direct3Dcontext, paramConstData[i], &paramConstBuffers[i]);
			}
		}

		direct3Dcontext->PSSetConstantBuffers(1, NUM_PARAM_BUFFERS, paramConstBuffers);
	}

	return true;
}

DX11::ComputeShader::ComputeShader(ID3D11Device * device) 
	: DX11::Shader(device, Type::Compute)
	, computeObject(0)
{
	ZeroMemory(paramConstBuffers, NUM_PARAM_BUFFERS * sizeof(void*));
}
DX11::ComputeShader::~ComputeShader()
{
	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		if(paramConstBuffers[i]) paramConstBuffers[i]->Release();
	}

	std::map<unsigned, ID3D11Buffer*>::iterator structBufferIt = paramStructBuffers.begin();
	for(; structBufferIt != paramStructBuffers.end(); ++structBufferIt)
	{
		if(structBufferIt->second) structBufferIt->second->Release();
	}

	if(computeObject) computeObject->Release();
}

bool DX11::ComputeShader::SetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect)
{
	direct3Dcontext->CSSetShader(computeObject, 0, 0);

	if(effect)
	{
		return SetExtraParameters(direct3Dcontext, effect);
	}
	else
	{
		return true;
	}
}

void DX11::ComputeShader::UnsetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect)
{
	unsigned numParams = effect->shader->paramSpecs.size();
	
	for(unsigned i = 0; i < paramMappings.size(); ++i)
	{
		ParamMapping & mapping = paramMappings[i];

		Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[mapping.paramIndex];
		Gpu::ShaderParam * param = effect->params[mapping.paramIndex];

		switch(spec.type)
		{
		case Gpu::ShaderParam::TypeParamBuffer:
		{
			if(mapping.writeable)
			{
				UINT nullCount = -1;
				ID3D11UnorderedAccessView * nullUav = 0;
				direct3Dcontext->CSSetUnorderedAccessViews(mapping.registerIndex, 1, &nullUav, &nullCount);
			}
			else
			{
				ID3D11ShaderResourceView * nullSrv = 0;
				direct3Dcontext->CSSetShaderResources(mapping.registerIndex, 1, &nullSrv);
			}
			break;
		}
		}
	}
}

bool DX11::ComputeShader::SetExtraParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect)
{
	unsigned numParams = effect->shader->paramSpecs.size();
	if(paramMappings.size() < numParams)
	{
		OutputDebugString(L"Not enough parameter mappings for the specified parameters!\n");
		return false;
	}

#if INTEL_HACK_COPYSTRUCTURECOUNT
	ZeroMemory(cpuImmutableConstBuffers, NUM_PARAM_BUFFERS * sizeof(bool));
#endif

	for(unsigned i = 0; i < paramMappings.size(); ++i)
	{
		ParamMapping & mapping = paramMappings[i];

		Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[mapping.paramIndex];
		Gpu::ShaderParam * param = effect->params[mapping.paramIndex];

		if(!param) return false;
		if(param->type != spec.type)
		{
			OutputDebugString(L"Parameter type does not match specified parameter type!\n");
			return false;
		}

#if INTEL_HACK_COPYSTRUCTURECOUNT
		if(cpuImmutableConstBuffers[mapping.registerIndex] &&
			(spec.type != Gpu::ShaderParam::TypeParamBuffer || mapping.attribute != BufferAttribute::Size))
		{
			OutputDebugString(L"Can only upload ParamBuffer sizes to this buffer!\n");
			return false;
		}
#endif

		switch(spec.type)
		{
		case Gpu::ShaderParam::TypeFloat:
		{
			std::vector<float> & constants = paramConstData[mapping.registerIndex];
			while(constants.size() <= mapping.bufferOffset)
			{
				constants.push_back(0.0f);
			}
			constants[mapping.bufferOffset] = param->fvalue;
			break;
		}
		case Gpu::ShaderParam::TypeFloatArray:
		{
			std::vector<float> & data = paramConstData[mapping.registerIndex];
			const unsigned minBufferSize = mapping.bufferOffset + param->avalue->numFloats;
			if(data.size() < minBufferSize) data.resize(minBufferSize);
			memcpy(&(data[mapping.bufferOffset]), param->avalue->floats, param->avalue->numFloats * sizeof(float));
			break;
		}
		case Gpu::ShaderParam::TypeParamBuffer:
		{
			DX11::ParamBuffer * paramBuffer = static_cast<DX11::ParamBuffer*>(param->bvalue);

			if(mapping.attribute == BufferAttribute::Size)
			{
				// Prepare the constant buffer, set nothing, wait until buffer has been created on GPU
				std::vector<float> & constants = paramConstData[mapping.registerIndex];
				while(constants.size() <= mapping.bufferOffset)
				{
					constants.push_back(0.0f);
				}

#if INTEL_HACK_COPYSTRUCTURECOUNT
				cpuImmutableConstBuffers[mapping.registerIndex] = true;
#endif
			}
			else
			{
				if(mapping.writeable)
				{
					// Set the structure count of the UAV
					direct3Dcontext->CSSetUnorderedAccessViews(mapping.registerIndex, 1, &paramBuffer->uav, &paramBuffer->uavCountToUpload);
					paramBuffer->uavCountToUpload = -1;
				}
				else
				{
					direct3Dcontext->CSSetShaderResources(mapping.registerIndex, 1, &paramBuffer->srv);
				}
			}
			break;
		}
		default:
			ApplyTextureParameter(direct3Dcontext, mapping.shader, mapping.registerIndex, param);
			break;
		}
	}

	if(numParams > 0)
	{
		// Upload CPU constants to GPU, creating buffers if necessary
		for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
		{
#if INTEL_HACK_COPYSTRUCTURECOUNT
			if(paramConstData[i].size() > 0 && (!cpuImmutableConstBuffers[i] || !paramConstBuffers[i]))
#else
			if(paramConstData[i].size() > 0)
#endif
			{
				UpdateConstantBuffer(direct3Dcontext, paramConstData[i], &paramConstBuffers[i]);
			}
		}

		// Overwrite GPU constants with GPU ParamBuffer sizes
		for(unsigned i = 0; i < paramMappings.size(); ++i)
		{
			ParamMapping & mapping = paramMappings[i];

			Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[mapping.paramIndex];
			Gpu::ShaderParam * param = effect->params[mapping.paramIndex];

			if(spec.type == Gpu::ShaderParam::TypeParamBuffer && mapping.attribute == BufferAttribute::Size)
			{
				DX11::ParamBuffer * paramBuffer = static_cast<DX11::ParamBuffer*>(param->bvalue);
				
				//{
				//	static ID3D11Buffer * debugBuf = 0;
				//	static unsigned debugData[2] = { 0 };

				//	if(!debugBuf)
				//	{
				//		ID3D11Device * device = 0;
				//		direct3Dcontext->GetDevice(&device);

				//		D3D11_SUBRESOURCE_DATA vbInitData;
				//		vbInitData.pSysMem = debugData;

				//		device->CreateBuffer(
				//			&CD3D11_BUFFER_DESC(2 * sizeof(unsigned), 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ),
				//			&vbInitData,
				//			&debugBuf);
				//	}

				//	direct3Dcontext->CopyStructureCount(debugBuf, 0 * sizeof(unsigned), paramBuffer->uav);

				//	D3D11_MAPPED_SUBRESOURCE mappedData = { 0 };
				//	direct3Dcontext->Map(debugBuf, 0, D3D11_MAP_READ, 0, &mappedData);
				//	memcpy(((char*)debugData), mappedData.pData, 2 * sizeof(unsigned));
				//	direct3Dcontext->Unmap(debugBuf, 0);

				//	std::wstringstream debugOutput;
				//	for(unsigned i = 0; i < 2; ++i) debugOutput << debugData[i] << ", ";
				//	debugOutput << std::endl;
				//	OutputDebugString(debugOutput.str().c_str());
				//}

				direct3Dcontext->CopyStructureCount(paramConstBuffers[mapping.registerIndex], 
					mapping.bufferOffset * sizeof(unsigned), paramBuffer->uav);

				//{
				//	static ID3D11Buffer * debugBuf = 0;
				//	static unsigned debugData[4] = { 0 };

				//	if(!debugBuf)
				//	{
				//		ID3D11Device * device = 0;
				//		direct3Dcontext->GetDevice(&device);

				//		D3D11_SUBRESOURCE_DATA vbInitData;
				//		vbInitData.pSysMem = debugData;

				//		device->CreateBuffer(
				//			&CD3D11_BUFFER_DESC(4 * sizeof(unsigned), 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ),
				//			&vbInitData,
				//			&debugBuf);
				//	}

				//	direct3Dcontext->CopyResource(debugBuf, paramConstBuffers[mapping.registerIndex]);

				//	D3D11_MAPPED_SUBRESOURCE mappedData = { 0 };
				//	direct3Dcontext->Map(debugBuf, 0, D3D11_MAP_READ, 0, &mappedData);
				//	memcpy(((char*)debugData), mappedData.pData, 4 * sizeof(unsigned));
				//	direct3Dcontext->Unmap(debugBuf, 0);

				//	std::wstringstream debugOutput;
				//	for(unsigned i = 0; i < 4; ++i) debugOutput << debugData[i] << ", ";
				//	debugOutput << std::endl;
				//	OutputDebugString(debugOutput.str().c_str());
				//}
			}
		}

		// Set the buffers into the pipeline
		direct3Dcontext->CSSetConstantBuffers(0, NUM_PARAM_BUFFERS, paramConstBuffers);
	}

	return true;
}

} // namespace Ingenuity

#endif // USE_DX11_GPUAPI
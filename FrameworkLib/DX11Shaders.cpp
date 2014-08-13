#ifdef USE_DX11_GPUAPI

#include "DX11Shaders.h"
#include "DX11Api.h"
#include "DX11Surfaces.h"
#include "tinyxml2.h"

using namespace DirectX;

namespace Ingenuity {

void DX11::Shader::ApplyTextureParameter(ID3D11DeviceContext * direct3Dcontext, unsigned registerIndex, Gpu::ShaderParam * param)
{
	if(!param) return;

	ID3D11ShaderResourceView * nullResource = 0;

	switch(param->type)
	{
	case Gpu::ShaderParam::TypeTexture:
	{
		DX11::Texture * texture = static_cast<Texture*>(param->tvalue);
		direct3Dcontext->PSSetShaderResources(registerIndex, 1, texture ? &texture->shaderView : &nullResource);
	}
		break;
	case Gpu::ShaderParam::TypeCubeTexture:
	{
		DX11::CubeMap * cubeMap = static_cast<CubeMap*>(param->cvalue);
		direct3Dcontext->PSSetShaderResources(registerIndex, 1, cubeMap ? &cubeMap->shaderView : &nullResource);
	}
		break;
	case Gpu::ShaderParam::TypeVolumeTexture:
	{
		DX11::VolumeTexture * volumeTex = static_cast<VolumeTexture*>(param->vvalue);
		direct3Dcontext->PSSetShaderResources(registerIndex, 1, volumeTex ? &volumeTex->shaderView : &nullResource);
	}
		break;
	case Gpu::ShaderParam::TypeDrawSurface:
	{
		DX11::Texture * texture = static_cast<Texture*>(param->svalue->GetTexture());
		direct3Dcontext->PSSetShaderResources(registerIndex, 1, texture ? &texture->shaderView : &nullResource);
	}
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

//XMMATRIX DX11::ModelShader::GetWorld(Gpu::Model * model)
//{
//	XMMATRIX w = XMMatrixIdentity();
//	w *= XMMatrixScaling(model->scale.x, model->scale.y, model->scale.z);
//	w *= XMMatrixRotationRollPitchYaw(model->rotation.x, model->rotation.y, model->rotation.z);
//	w *= XMMatrixTranslation(model->position.x, model->position.y, model->position.z);
//	return w;
//}

XMMATRIX DX11::ModelShader::GetView(Gpu::Camera * camera)
{
	const XMVECTOR pos = XMVectorSet(camera->position.x, camera->position.y, camera->position.z, 1.0f);
	const XMVECTOR target = XMVectorSet(camera->target.x, camera->target.y, camera->target.z, 1.0f);
	const XMVECTOR up = XMVectorSet(camera->up.x, camera->up.y, camera->up.z, 0.0f);
	return XMMatrixLookAtLH(pos, target, up);
}

XMMATRIX DX11::ModelShader::GetProjection(Gpu::Camera * camera, float aspect)
{
	return camera->isOrthoCamera ?
		XMMatrixOrthographicLH(camera->fovOrHeight * aspect, camera->fovOrHeight, camera->nearClip, camera->farClip) :
		XMMatrixPerspectiveFovLH(camera->fovOrHeight, aspect, camera->nearClip, camera->farClip);
}

DX11::ModelShader::Technique::Technique() :
inputLayout(0), vertexObject(0), pixelObject(0)
{
	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		vertexParamBuffers[i] = 0;
		pixelParamBuffers[i] = 0;
	}
}
DX11::ModelShader::Technique::~Technique()
{
	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		if(vertexParamBuffers[i]) vertexParamBuffers[i]->Release();
		if(pixelParamBuffers[i]) pixelParamBuffers[i]->Release();
	}

	if(inputLayout) inputLayout->Release();
	if(vertexObject) vertexObject->Release();
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

	ID3D11ShaderResourceView * nullResource = 0;

	for(unsigned i = 0; i < effect->shader->paramSpecs.size(); ++i)
	{
		ParamMapping & mapping = paramMappings[i];
		Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[i];
		Gpu::ShaderParam * param = effect->params[i];

		if(!param) return false;
		if(param->type != spec.type) return false;

		switch(spec.type)
		{
		case Gpu::ShaderParam::TypeFloat:
			switch(mapping.shader)
			{
			case ShaderStage::Vertex:
			{
				std::vector<float> & constants = vertexParamConstants[mapping.registerIndex - NUM_STANDARD_BUFFERS];
				while(constants.size() <= mapping.bufferOffset)
				{
					constants.push_back(0.0f);
				}
				constants[mapping.bufferOffset] = param->fvalue;
				break;
			}
			case ShaderStage::Pixel:
			{
				std::vector<float> & constants = pixelParamConstants[mapping.registerIndex - NUM_STANDARD_BUFFERS];
				while(constants.size() <= mapping.bufferOffset)
				{
					constants.push_back(0.0f);
				}
				constants[mapping.bufferOffset] = param->fvalue;
				break;
			}
			default:
				OutputDebugString(L"Non vertex/pixel shader states not yet implemented!\n");
				return false;
			}
			break;
		case Gpu::ShaderParam::TypeFloatArray:
			switch(mapping.shader)
			{
			case ShaderStage::Vertex:
			{
				std::vector<float> & constants = vertexParamConstants[mapping.registerIndex - NUM_STANDARD_BUFFERS];
				constants.resize(mapping.bufferOffset + param->avalue->numFloats + 1);
				memcpy(&(constants[mapping.bufferOffset]), param->avalue->floats, param->avalue->numFloats * sizeof(float));
			}
			case ShaderStage::Pixel:
			{
				std::vector<float> & constants = pixelParamConstants[mapping.registerIndex - NUM_STANDARD_BUFFERS];
				constants.resize(mapping.bufferOffset + param->avalue->numFloats + 1);
				memcpy(&(constants[mapping.bufferOffset]), param->avalue->floats, param->avalue->numFloats * sizeof(float));
			}
			default:
				OutputDebugString(L"Non vertex/pixel shader states not yet implemented!\n");
				return false;
			}
			break;
		default:
			ApplyTextureParameter(direct3Dcontext, mapping.registerIndex, param);
			break;
		}
	}

	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		if(vertexParamConstants[i].size() > 0)
		{
			UpdateConstantBuffer(direct3Dcontext, vertexParamConstants[i], &vertexParamBuffers[i]);
			direct3Dcontext->VSSetConstantBuffers(i + NUM_STANDARD_BUFFERS, 1, &vertexParamBuffers[i]);
		}

		if(pixelParamConstants[i].size() > 0)
		{
			UpdateConstantBuffer(direct3Dcontext, pixelParamConstants[i], &pixelParamBuffers[i]);
			direct3Dcontext->PSSetConstantBuffers(i + NUM_STANDARD_BUFFERS, 1, &pixelParamBuffers[i]);
		}
	}

	return true;
}

DX11::ModelShader::ModelShader(ID3D11Device * device) :
	DX11::Shader(device, true), vertexConstBuffer(0), pixelConstBuffer(0), currentTechnique(0)
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
}

bool DX11::ModelShader::SetTechnique(ID3D11DeviceContext * direct3Dcontext, VertexType vType, InstanceType iType)
{
	unsigned key = VertApi::GetTechniqueKey(vType, iType);

	std::map<unsigned, Technique>::iterator it = techniques.find(key);
	if(it == techniques.end())
	{
		OutputDebugString(L"Shader does not have technique for vertex/instance type!\n");
		return false;
	}

	currentTechnique = &(it->second);

	direct3Dcontext->IASetInputLayout(currentTechnique->inputLayout);
	direct3Dcontext->VSSetShader(currentTechnique->vertexObject, 0, 0);
	direct3Dcontext->PSSetShader(currentTechnique->pixelObject, 0, 0);

	return true;
}

bool DX11::ModelShader::SetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights, unsigned numLights, float aspect)
{
	if(!model || !camera) return false;
	if(camera->position == camera->target)
	{
		OutputDebugString(L"Cannot construct view matrix!");
		return false;
	}

	DX11::Mesh * dx11mesh = static_cast<DX11::Mesh*>(model->mesh);

	//XMMATRIX world = GetWorld(model);

	glm::mat4 modelMatrix = model->GetMatrix();
	XMMATRIX world((float*)&modelMatrix);

	XMStoreFloat4x4(&vertexConstData.world, world);

	XMMATRIX viewProj = GetView(camera) * GetProjection(camera, aspect);
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
		pixelConstData.ambient = 0.1f;
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

				//XMMATRIX lightView = XMMatrixLookToLH(
				//	XMLoadFloat3(&pixelConstData.lightPosition),
				//	XMLoadFloat3(&pixelConstData.spotDirection),
				//	XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f)));

				//float lightFOV = XM_PI * 0.10f;
				//XMMATRIX lightLens = XMMatrixPerspectiveFovLH(lightFOV, 1.0f, 1.0f, 200.0f);

				//XMStoreFloat4x4(&vertexConstData.lightViewProjection, lightView * lightLens);
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

	direct3Dcontext->UpdateSubresource(pixelConstBuffer, 0, 0, &pixelConstData, 0, 0);
	direct3Dcontext->PSSetConstantBuffers(0, 1, &pixelConstBuffer);

	direct3Dcontext->UpdateSubresource(vertexConstBuffer, 0, 0, &vertexConstData, 0, 0);
	direct3Dcontext->VSSetConstantBuffers(0, 1, &vertexConstBuffer);

	if(model->effect)
	{
		return currentTechnique->SetExtraParameters(direct3Dcontext, model->effect);
	}
	else
	{
		return true;
	}
}

ID3D11VertexShader * DX11::TextureShader::vertexObject = 0;

DX11::TextureShader::TextureShader(ID3D11Device * device) :
	DX11::Shader(device, false), standardConstBuffer(0), pixelObject(0)
{
	device->CreateBuffer(
		&CD3D11_BUFFER_DESC(sizeof(StandardConstants), D3D11_BIND_CONSTANT_BUFFER),
		0,
		&standardConstBuffer);

	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		paramBuffers[i] = 0;
	}
}
DX11::TextureShader::~TextureShader()
{
	for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
	{
		if(paramBuffers[i]) paramBuffers[i]->Release();
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

	for(unsigned i = 0; i < effect->shader->paramSpecs.size(); ++i)
	{
		ParamMapping & mapping = paramMappings[i];
		Gpu::ShaderParamSpec & spec = effect->shader->paramSpecs[i];
		Gpu::ShaderParam * param = effect->params[i];

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
			std::vector<float> & constants = paramConstants[mapping.registerIndex - 1];
			while(constants.size() <= mapping.bufferOffset)
			{
				constants.push_back(0.0f);
			}
			constants[mapping.bufferOffset] = param->fvalue;
			break;
		}
		case Gpu::ShaderParam::TypeFloatArray:
		{
			std::vector<float> & constants = paramConstants[mapping.registerIndex - 1];
			const unsigned minBufferSize = mapping.bufferOffset + param->avalue->numFloats;
			if(constants.size() < minBufferSize) constants.resize(minBufferSize);
			memcpy(&(constants[mapping.bufferOffset]), param->avalue->floats, param->avalue->numFloats * sizeof(float));
			break;
		}
		default:
			ApplyTextureParameter(direct3Dcontext, mapping.registerIndex, param);
			break;
		}
	}

	if(numParams > 0)
	{
		for(unsigned i = 0; i < NUM_PARAM_BUFFERS; ++i)
		{
			if(paramConstants[i].size() > 0)
			{
				UpdateConstantBuffer(direct3Dcontext, paramConstants[i], &paramBuffers[i]);
			}
		}

		direct3Dcontext->PSSetConstantBuffers(1, NUM_PARAM_BUFFERS, paramBuffers);
	}

	return true;
}

} // namespace Ingenuity

#endif // USE_DX11_GPUAPI
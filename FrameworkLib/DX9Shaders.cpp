#ifdef USE_DX9_GPUAPI

#include "DX9Shaders.h"
#include "DX9Surfaces.h"
#include <string>

bool DX9_GpuShader::SetTexture(D3DXHANDLE handle, GpuTexture * texture)
{
	if(!texture) return false;
	if(!handle) return false;

	DX9_GpuTexture * dx9tex = static_cast<DX9_GpuTexture*>(texture);
	return SUCCEEDED(shaderObject->SetTexture(handle, dx9tex->textureObject));
}

bool DX9_GpuShader::SetTexture(D3DXHANDLE handle, GpuCubeMap *cubeMap)
{
	if(!cubeMap) return false;
	if(!handle) return false;

	DX9_GpuCubeMap * dx9cubeMap = static_cast<DX9_GpuCubeMap*>(cubeMap);
	return SUCCEEDED(shaderObject->SetTexture(handle, dx9cubeMap->cubeMapObject));
}

bool DX9_GpuShader::SetTexture(D3DXHANDLE handle, GpuVolumeTexture * volumeTex)
{
	if(!volumeTex) return false;
	if(!handle) return false;

	DX9_GpuVolumeTexture * dx9volumeTex = static_cast<DX9_GpuVolumeTexture*>(volumeTex);
	return SUCCEEDED(shaderObject->SetTexture(handle, dx9volumeTex->textureObject));
}

bool DX9_GpuShader::SetTexture(D3DXHANDLE handle, GpuDrawSurface * surface)
{
	if(!surface) return false;
	if(!handle) return false;

	DX9_GpuDrawSurface * dx9surface = static_cast<DX9_GpuDrawSurface*>(surface);
	DX9_GpuTexture * dx9tex = static_cast<DX9_GpuTexture*>(dx9surface->GetTexture());
	return SUCCEEDED(shaderObject->SetTexture(handle, dx9tex->textureObject));
}

bool DX9_ModelShader::SetWorld(GpuModel * model, D3DXMATRIX * worldMatrix)
{
	D3DXMATRIX temp;
	D3DXMatrixIdentity(worldMatrix);
	D3DXMatrixIdentity(&temp);
	(*worldMatrix) *= *D3DXMatrixScaling(&temp,model->scale.x,model->scale.y,model->scale.z);
	(*worldMatrix) *= *D3DXMatrixRotationYawPitchRoll(&temp,model->rotation.y,model->rotation.x,model->rotation.z);
	(*worldMatrix) *= *D3DXMatrixTranslation(&temp,model->position.x,model->position.y,model->position.z);
	return SUCCEEDED(shaderObject->SetMatrix(world,worldMatrix));
}

bool DX9_ModelShader::SetWorldInverseTranspose(D3DXMATRIX * worldMatrix)
{
	D3DXMATRIX witMatrix; 
	D3DXMatrixInverse(&witMatrix,0,worldMatrix); 
	D3DXMatrixTranspose(&witMatrix,&witMatrix);
	return SUCCEEDED(shaderObject->SetMatrix(worldInverseTranspose,&witMatrix));
}

bool DX9_ModelShader::SetViewProjection(GpuCamera *camera, float aspect)
{
	D3DXMATRIX camView, camProjection;
	D3DXMatrixLookAtLH(&camView, 
		&D3DXVECTOR3(camera->position.x,camera->position.y,camera->position.z), 
		&D3DXVECTOR3(camera->target.x,camera->target.y,camera->target.z),
		&D3DXVECTOR3(camera->up.x,camera->up.y,camera->up.z));
	if(camera->isOrthoCamera)
	{
		D3DXMatrixOrthoLH(&camProjection,
			aspect * camera->fovOrHeight, camera->fovOrHeight, camera->nearClip, camera->farClip);
	}
	else
	{
		D3DXMatrixPerspectiveFovLH(&camProjection,
			camera->fovOrHeight, aspect, camera->nearClip, camera->farClip);
	}
	return SUCCEEDED(shaderObject->SetMatrix(viewProjection, &(camView*camProjection)));
}

bool DX9_GpuShader::ApplyExtraParameters(GpuEffect * effect)
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
		GpuShaderParamSpec & spec = effect->shader->paramSpecs[mapping.paramIndex];
		GpuShaderParam * param = effect->params[mapping.paramIndex];
		//sprintf_s(paramName,"param%d",i);

		if(!param) return false;
		if(param->type != spec.type) return false;
		if(!mapping.handle) return false;
		
		switch(spec.type)
		{
		case GpuShaderParam::Float:
			shaderObject->SetFloat(mapping.handle, param->fvalue);
			break;
		case GpuShaderParam::Texture:
			{
				DX9_GpuTexture * dx9tex = static_cast<DX9_GpuTexture*>(param->tvalue);
				shaderObject->SetTexture(paramMappings[i].handle, dx9tex ? dx9tex->textureObject : 0);
				break;
			}
		case GpuShaderParam::CubeTexture:
			{
				DX9_GpuCubeMap * dx9cube = static_cast<DX9_GpuCubeMap*>(param->cvalue);
				shaderObject->SetTexture(paramMappings[i].handle, dx9cube ? dx9cube->cubeMapObject : 0);
				break;
			}
		case GpuShaderParam::VolumeTexture:
			{
				DX9_GpuVolumeTexture * dx9volume = static_cast<DX9_GpuVolumeTexture*>(param->vvalue);
				shaderObject->SetTexture(paramMappings[i].handle, dx9volume ? dx9volume->textureObject : 0);
				break;
			}
		case GpuShaderParam::DrawSurface:
			//valid = SetTexture(paramName, param->svalue);
			break;
		}

		//if(!valid) return false;
	}

	return true;
}

bool DX9_ModelShader::SetParameters(GpuModel * model, GpuCamera * camera, GpuLight ** lights, int numLights, float aspect)
{
	if(!camera) return false;

	D3DXMATRIX world;
	SetWorld(model, &world);
	SetWorldInverseTranspose(&world);
	SetViewProjection(camera, aspect);

	shaderObject->SetValue(cameraPosition,&GLMToD3DX(camera->position),sizeof(D3DXVECTOR3));
	shaderObject->SetValue(materialColor,&GLMToD3DXCOLOR(model->color),sizeof(D3DXCOLOR));
	shaderObject->SetFloat(spotPower, 0.0f);
	shaderObject->SetValue(spotDirection,&D3DXVECTOR3(0.0f,0.0f,0.0f),sizeof(D3DXVECTOR3));
	shaderObject->SetFloat(cubeMapAlpha, 0.0f);

	const float DEFAULT_ATTEN = 0.0f;

	if(numLights > 0)
	{
		shaderObject->SetFloat(ambient, 0.05f);
		shaderObject->SetValue(lightColor,&GLMToD3DXCOLOR(lights[0]->color),sizeof(D3DXCOLOR));
		shaderObject->SetFloat(specularPower, model->specPower);

		GpuLightType lightType = lights[0]->GetType();
		if(lightType == GpuLightType_Point)
		{
			GpuPointLight* pointLight = static_cast<GpuPointLight*>(lights[0]);
			shaderObject->SetValue(lightPosition,&GLMToD3DX(pointLight->position),sizeof(D3DXVECTOR3));
			shaderObject->SetFloat(attenuation,pointLight->atten);
		}
		else if(lightType == GpuLightType_Spot)
		{
			GpuSpotLight* spotLight = static_cast<GpuSpotLight*>(lights[0]);
			shaderObject->SetValue(lightPosition,&GLMToD3DX(spotLight->position),sizeof(D3DXVECTOR3));
			shaderObject->SetFloat(attenuation,spotLight->atten);
			shaderObject->SetValue(spotDirection,&GLMToD3DX(spotLight->direction),sizeof(D3DXVECTOR3));
			shaderObject->SetFloat(spotPower,spotLight->power);
		}
		else // GpuLightType_Directional
		{
			GpuDirectionalLight* dirLight = static_cast<GpuDirectionalLight*>(lights[0]);
			const float scale = 10.0e+10f;
			shaderObject->SetValue(lightPosition,&GLMToD3DX(dirLight->direction * scale),sizeof(D3DXVECTOR3));
			shaderObject->SetFloat(attenuation, DEFAULT_ATTEN);
		}
	}
	else
	{
		shaderObject->SetValue(lightPosition,&D3DXVECTOR3(0.0f,0.0f,0.0f),sizeof(D3DXVECTOR3));
		shaderObject->SetValue(lightColor,&D3DXCOLOR(1.0f,1.0f,1.0f,1.0f),sizeof(D3DXCOLOR));
		shaderObject->SetFloat(specularPower,FLT_MAX);
		shaderObject->SetFloat(ambient,1.0f);
		shaderObject->SetFloat(attenuation, DEFAULT_ATTEN);
	}

	SetTexture(tex, model->texture);
	SetTexture(normalMap, model->normalMap);

	if(model->cubeMap)
	{
		SetTexture(cubeMap, model->cubeMap);
		shaderObject->SetFloat(cubeMapAlpha, 1.0f);
	}
	else
	{
		SetTexture(cubeMap, (GpuCubeMap*) 0);
		shaderObject->SetFloat(cubeMapAlpha, 0.0f);
	}

	bool success = true;

	if(model->effect) success = ApplyExtraParameters(model->effect);
	if(success) success = SUCCEEDED(shaderObject->CommitChanges());

	return success;
}

bool DX9_ModelShader::SetTechnique(VertexType type, InstanceType iType)
{
	unsigned techniqueKey = VertApi::GetTechniqueKey(type,iType);
	std::map<unsigned,DX9_ModelShader::Technique>::iterator it = techniques.find(techniqueKey);
	if(it == techniques.end())
	{
		OutputDebugString(L"Shader does not have technique for vertex/instance type!\n");
		return false;
	}
	return SUCCEEDED(shaderObject->SetTechnique(it->second.handle));
}

bool DX9_TextureShader::SetParameters(GpuTexture * texture, GpuEffect * effect)
{
	if(!texture || !effect) return false;
	DX9_GpuTexture * dx9tex = static_cast<DX9_GpuTexture*>(texture);

	if(!SUCCEEDED(shaderObject->SetTechnique(technique))) return false;
	
	SetTexture(tex,dx9tex);
	shaderObject->SetFloat(texWidth,(float)dx9tex->width);
	shaderObject->SetFloat(texHeight,(float)dx9tex->height);

	return ApplyExtraParameters(effect);
}

#endif // USE_DX9_GPUAPI

#pragma once

#include "GpuShaders.h"
#include "DX9Api.h"
#include <string>
#include <vector>

struct DX9_GpuShader : public GpuShader, IGpuDeviceListener
{
	struct ParamMapping
	{
		unsigned paramIndex;
		std::string paramName;
		D3DXHANDLE handle;
	};

	std::vector<ParamMapping> paramMappings;

	GpuApi * gpu;
	ID3DXEffect * const shaderObject;

	virtual void OnLostDevice(GpuApi * gpu) override { shaderObject->OnLostDevice(); }
	virtual void OnResetDevice(GpuApi * gpu) override { shaderObject->OnResetDevice(); }

	virtual ~DX9_GpuShader()
	{
		if(shaderObject) shaderObject->Release();
		gpu->RemoveDeviceListener(this);
	}

protected:
	DX9_GpuShader(GpuApi * gpu, ID3DXEffect * object, bool modelShader) 
		: GpuShader(modelShader), gpu(gpu), shaderObject(object) 
	{
		gpu->AddDeviceListener(this);
	}

	bool SetTexture(D3DXHANDLE handle, GpuTexture *texture);
	bool SetTexture(D3DXHANDLE handle, GpuCubeMap *cubeMap);
	bool SetTexture(D3DXHANDLE handle, GpuVolumeTexture * volumeTex);
	bool SetTexture(D3DXHANDLE handle, GpuDrawSurface * surface);

	D3DXVECTOR3 GLMToD3DX(glm::vec3 vec) { return D3DXVECTOR3(vec.x,vec.y,vec.z); }
	D3DXCOLOR GLMToD3DXCOLOR(glm::vec4 col) { return D3DXCOLOR(col.r,col.g,col.b,col.a); }
	D3DXCOLOR GLMToD3DXCOLOR(glm::vec3 col) { return D3DXCOLOR(col.r,col.g,col.b,1.0f); }

	bool ApplyExtraParameters(GpuEffect * effect);
};

struct DX9_ModelShader : public DX9_GpuShader
{
	struct Technique
	{
		D3DXHANDLE handle;
		IDirect3DVertexDeclaration9 * declaration;

		Technique() : handle(0), declaration(0) {}
		~Technique()
		{
			if(declaration) declaration->Release();
		}
	};

	std::map<unsigned,Technique> techniques;

	// Vertex Parameters
	D3DXHANDLE world;
	D3DXHANDLE worldInverseTranspose;
	D3DXHANDLE viewProjection;

	// Pixel Parameters
	D3DXHANDLE cameraPosition;
	D3DXHANDLE materialColor;
	D3DXHANDLE spotPower;
	D3DXHANDLE spotDirection;
	D3DXHANDLE cubeMapAlpha;
	D3DXHANDLE ambient;
	D3DXHANDLE lightColor;
	D3DXHANDLE specularPower;
	D3DXHANDLE lightPosition;
	D3DXHANDLE attenuation;
	D3DXHANDLE tex;
	D3DXHANDLE cubeMap;
	D3DXHANDLE normalMap;

	DX9_ModelShader(GpuApi * gpu, ID3DXEffect * object)
		: DX9_GpuShader(gpu, object, true)
	{
		world = object->GetParameterByName(0,"world");
		worldInverseTranspose = object->GetParameterByName(0,"worldInverseTranspose");
		viewProjection = object->GetParameterByName(0,"viewProjection");

		cameraPosition = object->GetParameterByName(0,"cameraPosition");
		materialColor = object->GetParameterByName(0,"materialColor");
		spotPower = object->GetParameterByName(0,"spotPower");
		spotDirection = object->GetParameterByName(0,"spotDirection");
		cubeMapAlpha = object->GetParameterByName(0,"cubeMapAlpha");
		ambient = object->GetParameterByName(0,"ambient");
		lightColor = object->GetParameterByName(0,"lightColor");
		specularPower = object->GetParameterByName(0,"specularPower");
		lightPosition = object->GetParameterByName(0,"lightPosition");
		attenuation = object->GetParameterByName(0,"attenuation");
		tex = object->GetParameterByName(0,"tex");
		cubeMap = object->GetParameterByName(0,"cubeMap");
		normalMap = object->GetParameterByName(0,"normalMap");
	}
	virtual ~DX9_ModelShader() {}

	bool SetWorld(GpuModel * model, D3DXMATRIX * world);
	bool SetWorldInverseTranspose(D3DXMATRIX * world);
	bool SetViewProjection(GpuCamera * camera, float aspect);

	virtual bool SetTechnique(VertexType type, InstanceType iType);
	virtual bool SetParameters(GpuModel * model, GpuCamera * camera, GpuLight ** lights, int numLights, float aspect);
};

struct DX9_TextureShader : public DX9_GpuShader
{
	D3DXHANDLE technique;
	D3DXHANDLE tex;
	D3DXHANDLE texWidth;
	D3DXHANDLE texHeight;

	DX9_TextureShader(GpuApi * gpu, ID3DXEffect * object) 
		: DX9_GpuShader(gpu, object, false) 
	{
		technique = object->GetTechniqueByName("posTex");
		tex = object->GetParameterByName(0,"tex");
		texWidth = object->GetParameterByName(0,"texWidth");
		texHeight = object->GetParameterByName(0,"texHeight");
	}
	virtual ~DX9_TextureShader() {}

	virtual bool SetParameters(GpuTexture * texture, GpuEffect * effect);
};

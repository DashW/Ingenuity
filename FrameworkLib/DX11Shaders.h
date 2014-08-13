#pragma once

#include "GpuShaders.h"
#include "GpuVertices.h"
#include <DirectXMath.h>

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
#include <d3d11.h>
#else
#include <d3d11_1.h>
#endif

#include <map>
#include <vector>

namespace Ingenuity {
namespace DX11 {

struct Shader : public Gpu::Shader
{
	enum ShaderStage
	{
		Vertex,
		Geometry,
		Pixel
	};

	struct ParamMapping
	{
		ShaderStage shader;
		unsigned registerIndex;
		unsigned bufferOffset;
	};

	typedef std::map<unsigned, ParamMapping> ParamMap;

	Shader(ID3D11Device * device, bool modelShader) : Gpu::Shader(modelShader) {}
	virtual ~Shader() {}

	static void ApplyTextureParameter(ID3D11DeviceContext * context, unsigned registerIndex, Gpu::ShaderParam * param);

	static void UpdateConstantBuffer(ID3D11DeviceContext * context, std::vector<float> & constants, ID3D11Buffer ** buffer);
};

struct ModelShader : public Shader
{
	static const unsigned NUM_STANDARD_BUFFERS = 2;
	static const unsigned NUM_PARAM_BUFFERS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - NUM_STANDARD_BUFFERS;
	static const unsigned MAX_LIGHTS = 6;

	struct Technique
	{
		ID3D11InputLayout * inputLayout;
		ID3D11VertexShader * vertexObject;
		ID3D11PixelShader * pixelObject; // OH WOW. MAYBE VERTEX & PIXEL SHADERS NEED TO BE PUT IN THEIR OWN BANKS...

		ParamMap paramMappings;
		std::vector<float> pixelParamConstants[NUM_PARAM_BUFFERS];
		std::vector<float> vertexParamConstants[NUM_PARAM_BUFFERS];
		ID3D11Buffer * pixelParamBuffers[NUM_PARAM_BUFFERS];
		ID3D11Buffer * vertexParamBuffers[NUM_PARAM_BUFFERS];

		Technique();
		~Technique();

		bool SetExtraParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect);

		void ApplySamplerParams(ID3D11Device * device, ID3D11DeviceContext * context, std::vector<Gpu::SamplerParam> & samplerParams);
	};

	struct VertexConstants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 viewProjection;
		DirectX::XMFLOAT4X4 worldInverseTranspose;
		DirectX::XMFLOAT4X4 lightViewProjection;
		DirectX::XMFLOAT4 materialColor;

		VertexConstants() :
			materialColor(0.0f, 0.0f, 0.0f, 0.0f) {}
	};

	struct PixelConstants
	{
		DirectX::XMFLOAT3 lightPosition;
		float specularPower;
		DirectX::XMFLOAT4 lightColor;
		DirectX::XMFLOAT3 cameraPosition;
		float ambient;
		DirectX::XMFLOAT3 spotDirection;
		float spotPower;
		float cubeMapAlpha;
		unsigned numLights;
		float specularFactor;
		float diffuseFactor;

		PixelConstants() :
			lightPosition(0.0f, 0.0f, 0.0f), specularPower(0.0f), lightColor(0.0f, 0.0f, 0.0f, 0.0f),
			cameraPosition(0.0f, 0.0f, 0.0f), ambient(0.0f), spotDirection(0.0f, 0.0f, 0.0f), spotPower(0.0f),
			cubeMapAlpha(0.0f), numLights(0), specularFactor(1.0f), diffuseFactor(0.0f) {}
	};

	struct LightConstants
	{
		struct
		{
			DirectX::XMFLOAT3 position;
			float specPower;
		}
		positionSpecs[MAX_LIGHTS];
		struct
		{
			DirectX::XMFLOAT3 color;
			float attenuation;
		}
		colorAttenuations[MAX_LIGHTS];
		struct
		{
			DirectX::XMFLOAT3 direction;
			float spotPower;
		}
		spotDirPowers[MAX_LIGHTS];

		LightConstants()
		{
			for(unsigned i = 0; i < MAX_LIGHTS; ++i)
			{
				positionSpecs[i].position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				positionSpecs[i].specPower = 0.0f;
				colorAttenuations[i].color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				colorAttenuations[i].attenuation = 0.0f;
				spotDirPowers[i].direction = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				spotDirPowers[i].spotPower = 0.0f;
			}
		}
	};

	VertexConstants vertexConstData;
	PixelConstants pixelConstData;
	LightConstants lightConstData;

	ID3D11Buffer * vertexConstBuffer;
	ID3D11Buffer * pixelConstBuffer;
	ID3D11Buffer * lightParamsBuffer;

	std::map<unsigned, Technique> techniques;
	Technique * currentTechnique;

	ModelShader(ID3D11Device * device);
	virtual ~ModelShader();

	//DirectX::XMMATRIX GetWorld(Gpu::Model * model);
	DirectX::XMMATRIX GetView(Gpu::Camera * camera);
	DirectX::XMMATRIX GetProjection(Gpu::Camera * camera, float aspect);

	bool SetTechnique(ID3D11DeviceContext * direct3Dcontext, VertexType vType, InstanceType iType);
	bool SetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights, unsigned numLights, float aspect);
};

struct TextureShader : public Shader
{
	struct StandardConstants
	{
		float texWidth;
		float texHeight;
		DirectX::XMFLOAT2 filler;

		StandardConstants() : texWidth(0.0f), texHeight(0.0f) {}
	};

	StandardConstants standardConstData;
	ID3D11Buffer * standardConstBuffer;

	ParamMap paramMappings;
	static const unsigned NUM_PARAM_BUFFERS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1;
	std::vector<float> paramConstants[NUM_PARAM_BUFFERS];
	ID3D11Buffer * paramBuffers[NUM_PARAM_BUFFERS];

	static ID3D11VertexShader * vertexObject;
	ID3D11PixelShader * pixelObject;

	TextureShader(ID3D11Device * device);
	virtual ~TextureShader();

	virtual bool SetParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Texture * texture, Gpu::Effect * effect);

protected:
	bool SetExtraParameters(ID3D11DeviceContext * direct3Dcontext, Gpu::Effect * effect);
};

} // namespace DX11
} // namespace Ingenuity

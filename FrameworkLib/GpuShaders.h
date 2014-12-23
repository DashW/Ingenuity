#pragma once

#include "AssetMgr.h"

#include <vector>

namespace Ingenuity {
namespace Gpu {

struct FloatArray
{
	float * floats;
	unsigned numFloats;

	FloatArray(float * floats, unsigned numFloats) :
		floats(floats), numFloats(numFloats) {}
};

struct Texture;
struct CubeMap;
struct VolumeTexture;
struct DrawSurface;

struct ShaderParam
{
	enum Type
	{
		TypeInteger,
		TypeFloat,
		//TypeMatrix,
		TypeFloatArray,
		TypeTexture,
		TypeCubeTexture,
		TypeVolumeTexture,
		TypeDrawSurface,
		//TypeBlob
	} type;

	union
	{
		int ivalue;
		float fvalue;
		//glm::mat4x4 * mvalue;
		FloatArray * avalue;
		Texture * tvalue;
		CubeMap * cvalue;
		VolumeTexture * vvalue;
		DrawSurface * svalue;
		//Blob * bvalue;
	};

	ShaderParam(int value)
		: type(TypeInteger), ivalue(value) {}
	ShaderParam(float value)
		: type(TypeFloat), fvalue(value) {}
	ShaderParam(Texture * value)
		: type(TypeTexture), tvalue(value) {}
	ShaderParam(CubeMap * value)
		: type(TypeCubeTexture), cvalue(value) {}
	ShaderParam(VolumeTexture * value)
		: type(TypeVolumeTexture), vvalue(value) {}
	ShaderParam(DrawSurface * value)
		: type(TypeDrawSurface), svalue(value) {}
	ShaderParam(FloatArray * value)
		: type(TypeFloatArray), avalue(value) {}
	ShaderParam(Type type)
		: type(type), tvalue(0) {}

	void Set(int value) { if(type == TypeInteger) ivalue = value; }
	void Set(float value) { if(type == TypeFloat) fvalue = value; }
	void Set(Texture * value) { if(type == TypeTexture) tvalue = value; }
	void Set(CubeMap * value) { if(type == TypeCubeTexture) cvalue = value; }
	void Set(VolumeTexture * value) { if(type == TypeVolumeTexture) vvalue = value; }
	void Set(DrawSurface * value) { if(type == TypeDrawSurface) svalue = value; }
	void Set(FloatArray * value) { if(type == TypeFloatArray) avalue = value; }
};

struct ShaderParamSpec
{
	std::string displayName;

	ShaderParam::Type type;

	float defaultFValue;

	bool ranged;
	float rangeMin;
	float rangeMax;

	ShaderParamSpec() :
		type(ShaderParam::TypeFloat),
		defaultFValue(0.0f),
		ranged(false) {}
};

struct SamplerParam
{
	static const unsigned DEFAULT_SAMPLER = UINT_MAX;

	unsigned paramIndex;

	enum ParamKey
	{
		AddressU,
		AddressV,
		AddressW,

		Filter,
		Anisotropy,
		Comparison,

		KeyMAX
	}
	key;

	enum AddressMode
	{
		AddressWrap,
		AddressMirror,
		AddressClamp,

		AddressModeMAX
	};

	enum FilterMode
	{
		FilterGlobal,
		FilterNone,
		FilterPoint,
		FilterLinear,
		FilterAnisotropic,

		FilterModeMAX
	};

	enum ComparisonFunc
	{
		ComparisonNone,
		ComparisonLess,
		ComparisonGreater,
		ComparisonAlways,
		ComparisonNever,

		ComparisonMAX
	};

	unsigned value;

	SamplerParam(unsigned paramIndex, ParamKey key, unsigned value)
		: paramIndex(paramIndex), key(key), value(value) {}
};

struct Shader : public IAsset
{
	enum BlendMode
	{
		Add,
		Sub,
		Rsub,
		Mul,
		Max,
		Min,
		None
	} blendMode;

	std::vector<ShaderParamSpec> paramSpecs;
	std::vector<SamplerParam> defaultSamplerParams;

	bool IsModelShader() const { return modelShader; }
	bool IsTextureShader() const { return !modelShader; }

	virtual ~Shader() {}

	virtual AssetType GetType() override { return ShaderAsset; }
	virtual IAsset * GetAsset() override { return this; }

protected:
	const bool modelShader;

	Shader(bool modelShader) : blendMode(None), modelShader(modelShader) {}
};

struct Effect
{
	Shader * shader;
	ShaderParam ** const params;
	std::vector<SamplerParam> samplerParams;

	Effect(Shader * shader) :
		shader(shader),
		params(new ShaderParam*[shader->paramSpecs.size()])
	{
		for(unsigned i = 0; i < shader->paramSpecs.size(); ++i)
		{
			if(shader->paramSpecs[i].type == ShaderParam::TypeFloat)
			{
				params[i] = new ShaderParam(shader->paramSpecs[i].defaultFValue);
			}
			else
			{
				params[i] = new ShaderParam(shader->paramSpecs[i].type);
			}
		}
		samplerParams = shader->defaultSamplerParams;
	}
	~Effect()
	{
		for(unsigned i = 0; i < shader->paramSpecs.size(); ++i)
		{
			if(params[i]) delete params[i];
		}
		delete[] params;
	}

	template<typename T>
	void SetParam(unsigned index, T value)
	{
		if(shader->paramSpecs.size() <= index) return;
		if(params[index] != 0)
		{
			params[index]->Set(value);
		}
		else
		{
			params[index] = new Gpu::ShaderParam(value);
		}
	}

	void SetSamplerParam(SamplerParam::ParamKey key, unsigned value, unsigned paramIndex = SamplerParam::DEFAULT_SAMPLER)
	{
		for(unsigned i = 0; i < samplerParams.size(); ++i)
		{
			if(samplerParams[i].key == key && samplerParams[i].paramIndex == paramIndex)
			{
				samplerParams[i].value = value;
				return;
			}
		}
		samplerParams.emplace_back(paramIndex, key, value);
	}
};

} // namespace Gpu
} // namespace Ingenuity

#include "DX11Samplers.h"

namespace Ingenuity {

DX11::SamplerMgr::SamplerMgr(ID3D11Device * direct3Ddevice, ID3D11DeviceContext * direct3Dcontext) : 
	direct3Ddevice(direct3Ddevice)
{
	FillMemory(&currentSamplerKeys, sizeof(unsigned)*D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, 0xf);

	D3D11_SAMPLER_DESC samDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ID3D11SamplerState * defaultSampler = 0;
	direct3Ddevice->CreateSamplerState(&samDesc, &defaultSampler);
	if(defaultSampler) createdSamplerStates[0] = defaultSampler;

	for(unsigned i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++i)
	{
		direct3Dcontext->PSSetSamplers(i, 1, &defaultSampler);
	}
}

DX11::SamplerMgr::~SamplerMgr()
{
	SamplerBank::iterator samplerIt = createdSamplerStates.begin();
	for(; samplerIt != createdSamplerStates.end(); ++samplerIt)
	{
		samplerIt->second->Release();
	}
}

void DX11::SamplerMgr::UpdateSamplerKey(unsigned & samplerKey, Gpu::SamplerParam & param)
{
	if(samplerKey == UINT_MAX) samplerKey = 0;

	unsigned value = param.value;
	unsigned upper = 0;
	unsigned lower = 0;

	switch(param.key)
	{
	case Gpu::SamplerParam::AddressU:
		upper = ADDRESS_V;
		lower = ADDRESS_U;
		break;
	case Gpu::SamplerParam::AddressV:
		upper = ADDRESS_W;
		lower = ADDRESS_V;
		break;
	case Gpu::SamplerParam::AddressW:
		upper = FILTER_MD;
		lower = ADDRESS_W;
		break;
	case Gpu::SamplerParam::Filter:
		upper = MAX_ANISO;
		lower = FILTER_MD;
		break;
	case Gpu::SamplerParam::Anisotropy:
		upper = COMP_FUNC;
		lower = MAX_ANISO;
		break;
	case Gpu::SamplerParam::Comparison:
		upper = MIPMAPOFF;
		lower = COMP_FUNC;
		break;
	case Gpu::SamplerParam::MipDisable:
		upper = MAX__BITS;
		lower = MIPMAPOFF;
		break;
	}

	// Clear the bits
	samplerKey &= ~GetBitmask(upper, lower);

	// Set the new value
	samplerKey |= value << lower;
}

void DX11::SamplerMgr::ApplySamplerParams(
	ID3D11DeviceContext * direct3Dcontext, 
	DX11::Shader::ParamMap & paramMappings, 
	std::vector<Gpu::SamplerParam> * samplerParams, 
	bool isModelShader, 
	bool forced)
{
	unsigned samplerKeys[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	ZeroMemory(&samplerKeys, sizeof(unsigned)*D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

	if(samplerParams)
	{
		for(unsigned i = 0; i < samplerParams->size(); ++i)
		{
			Gpu::SamplerParam & param = samplerParams->at(i);
			if(param.paramIndex < paramMappings.size())
			{
				unsigned samplerSlot = paramMappings[param.paramIndex].registerIndex;

				if(samplerSlot >= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) continue;
				UpdateSamplerKey(samplerKeys[samplerSlot], param);
			}
			else if(param.paramIndex == Gpu::SamplerParam::DEFAULT_SAMPLER)
			{
				UpdateSamplerKey(samplerKeys[0], param);
				if(isModelShader) UpdateSamplerKey(samplerKeys[2], param);
			}
		}
	}

	static const D3D11_TEXTURE_ADDRESS_MODE GPU_TEXTURE_MODES_TO_DX11_TEXTURE_MODES[] = {
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_CLAMP
	};

	static const D3D11_FILTER GPU_FILTER_MODES_TO_DX11_FILTERS[] = {
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, // FIXME: This needs to be the inherited global filtering mode
		D3D11_FILTER_MIN_MAG_MIP_LINEAR, // No option for 'none', default to linear
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_ANISOTROPIC
	};

	static const D3D11_FILTER GPU_FILTER_MODES_TO_DX11_COMPARISON_FILTERS[] = {
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_COMPARISON_ANISOTROPIC
	};

	static const D3D11_COMPARISON_FUNC GPU_COMPARISON_FUNCS_TO_DX11_COMPARISON_FUNCS[] = {
		(D3D11_COMPARISON_FUNC)0,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_ALWAYS,
		D3D11_COMPARISON_NEVER
	};

	for(unsigned i = 0; i < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; ++i)
	{
		// Important! This allows sampler params to have an amortised cost.
		if(currentSamplerKeys[i] == samplerKeys[i] && !forced) continue;

		ID3D11SamplerState * samplerState = 0;
		unsigned samplerKey = samplerKeys[i];

		SamplerBank::iterator it = createdSamplerStates.find(samplerKey);
		if(it != createdSamplerStates.end())
		{
			samplerState = it->second;
		}
		else
		{
			D3D11_SAMPLER_DESC samDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
			samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

			unsigned addressU = (samplerKey & GetBitmask(ADDRESS_V, ADDRESS_U)) >> ADDRESS_U;
			samDesc.AddressU = GPU_TEXTURE_MODES_TO_DX11_TEXTURE_MODES[addressU];

			unsigned addressV = (samplerKey & GetBitmask(ADDRESS_W, ADDRESS_V)) >> ADDRESS_V;
			samDesc.AddressV = GPU_TEXTURE_MODES_TO_DX11_TEXTURE_MODES[addressV];

			unsigned addressW = (samplerKey & GetBitmask(FILTER_MD, ADDRESS_W)) >> ADDRESS_W;
			samDesc.AddressW = GPU_TEXTURE_MODES_TO_DX11_TEXTURE_MODES[addressW];

			unsigned anisotropy = (samplerKey & GetBitmask(COMP_FUNC, MAX_ANISO)) >> MAX_ANISO;
			samDesc.MaxAnisotropy = anisotropy;

			unsigned comparisonFunc = (samplerKey & GetBitmask(MIPMAPOFF, COMP_FUNC)) >> COMP_FUNC;
			samDesc.ComparisonFunc = GPU_COMPARISON_FUNCS_TO_DX11_COMPARISON_FUNCS[comparisonFunc];

			unsigned filterMode = (samplerKey & GetBitmask(MAX_ANISO, FILTER_MD)) >> FILTER_MD;
			if(comparisonFunc == Gpu::SamplerParam::ComparisonNone)
				samDesc.Filter = GPU_FILTER_MODES_TO_DX11_FILTERS[filterMode];
			else
				samDesc.Filter = GPU_FILTER_MODES_TO_DX11_COMPARISON_FILTERS[filterMode];

			unsigned mipDisabled = (samplerKey & GetBitmask(MAX__BITS, MIPMAPOFF)) >> MIPMAPOFF;
			if(mipDisabled > 0) samDesc.MinLOD = samDesc.MaxLOD = 0.0f;

			if(SUCCEEDED(direct3Ddevice->CreateSamplerState(&samDesc, &samplerState)))
			{
				createdSamplerStates[samplerKeys[i]] = samplerState;
			}
		}

		direct3Dcontext->PSSetSamplers(i, 1, &samplerState);
		currentSamplerKeys[i] = samplerKeys[i];
	}
}

void DX11::SamplerMgr::SetAnisotropy(
	ID3D11DeviceContext * direct3Dcontext, 
	unsigned anisotropy)
{
	D3D11_SAMPLER_DESC samDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	if(anisotropy > 0)
	{
		samDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samDesc.MaxAnisotropy = anisotropy;
	}
	else
	{
		samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}

	ID3D11SamplerState * defaultSampler = 0;
	direct3Ddevice->CreateSamplerState(&samDesc, &defaultSampler);

	if(defaultSampler)
	{
		createdSamplerStates[0]->Release();
		createdSamplerStates[0] = defaultSampler;
	}

	for(unsigned i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++i)
	{
		direct3Dcontext->PSSetSamplers(i, 1, &defaultSampler);
	}
}

void DX11::SamplerMgr::ResetSamplerParams()
{
	ZeroMemory(&currentSamplerKeys, sizeof(unsigned)*D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
}

} // namespace Ingenuity

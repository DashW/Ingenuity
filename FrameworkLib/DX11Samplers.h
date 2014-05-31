#pragma once

#include "DX11Shaders.h"

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
#include <d3d11.h>
#else
#include <d3d11_1.h>
#endif

#include <map>

namespace Ingenuity {
namespace DX11 {

class SamplerMgr
{
	typedef std::map<unsigned, ID3D11SamplerState*> SamplerBank;

	SamplerBank createdSamplerStates;
	unsigned currentSamplerKeys[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

	ID3D11Device * direct3Ddevice;

	static const unsigned ADDRESS_U = 1;
	static const unsigned ADDRESS_V = ADDRESS_U * Gpu::SamplerParam::AddressModeMAX;
	static const unsigned ADDRESS_W = ADDRESS_V * Gpu::SamplerParam::AddressModeMAX;
	static const unsigned FILTER_MD = ADDRESS_W * Gpu::SamplerParam::AddressModeMAX;
	static const unsigned MAX_ANISO = FILTER_MD * Gpu::SamplerParam::FilterModeMAX;

	void UpdateSamplerKey(unsigned & samplerKey, Gpu::SamplerParam & param);

public:
	SamplerMgr(ID3D11Device * direct3Ddevice, ID3D11DeviceContext * direct3Dcontext);
	~SamplerMgr();

	void ApplySamplerParams(
		ID3D11DeviceContext * direct3Dcontext, 
		DX11::Shader::ParamMap & paramMappings, 
		std::vector<Gpu::SamplerParam> * samplerParams = 0, 
		bool isModelShader = true);

	void SetAnisotropy(
		ID3D11DeviceContext * direct3Dcontext, 
		unsigned anisotropy);
};

} // namespace DX11
} // namespace Ingenuity

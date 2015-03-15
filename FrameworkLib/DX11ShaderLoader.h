#pragma once

#include "ShaderParser.h"
#include "DX11Shaders.h"
#include "tinyxml2.h"

namespace Ingenuity {
namespace DX11 {

class Api;

class ShaderLoader : public Gpu::ShaderLoader
{
	Api * gpu;

	enum ShaderLoadState
	{
		LOADING,
		SUCCEEDED,
		FAILED
	};

	struct VertexShaderResponse;
	struct GeometryShaderResponse;
	struct PixelShaderResponse;
	struct ComputeShaderResponse;

	struct PendingTechnique
	{
		unsigned key;
		VertexShaderResponse * vertexResponse;
		GeometryShaderResponse * geometryResponse;
		PixelShaderResponse * pixelResponse;

		PendingTechnique(unsigned key)
			: key(key)
			, vertexResponse(0)
			, geometryResponse(0)
			, pixelResponse(0)
		{}
	};

	static const char* primitiveStrings[Shader::PrimitiveCount];

	std::vector<PendingTechnique> pendingTechniques;
	ComputeShaderResponse * pendingComputeResponse;

	bool ParseParamMappingsXML(tinyxml2::XMLElement * element, DX11::Shader * shader, DX11::Shader::ParamMap & paramMappings);
	DX11::ModelShader * ParseModelShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser);
	DX11::TextureShader * ParseTextureShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser);
	DX11::ComputeShader * ParseComputeShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser);
	void DeleteResponses();

public:
	ShaderLoader(Api * gpu, Files::Api * files, Files::Directory * directory, const wchar_t * path);
	virtual ~ShaderLoader();

	void ParseXml() override
	{
		if(!parser) { loaderFailed = true; return; }
		tinyxml2::XMLElement * targetApiElement = parser->GetApiElement("directx11");
		if(!targetApiElement) { loaderFailed = true; return; }
		if(parser->IsModelShader())
		{
			asset = ParseModelShaderXML(targetApiElement, parser);
		}
		else if(parser->IsComputeShader())
		{
			asset = ParseComputeShaderXML(targetApiElement, parser);
		}
		else
		{
			asset = ParseTextureShaderXML(targetApiElement, parser);
		}
		if(!asset) loaderFailed = true;
	}
	bool IsFinished() override;
};

} // namespace DX11
} // namespace Ingenuity

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
	unsigned numVertexShaders;
	unsigned numPixelShaders;
	unsigned vertexShadersLoaded;
	unsigned pixelShadersLoaded;

	struct VertexShaderResponse;
	struct PixelShaderResponse;

	bool ParseParamMappingsXML(tinyxml2::XMLElement * element, DX11::Shader * shader, DX11::Shader::ParamMap & paramMappings);
	DX11::ModelShader * ParseModelShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser);
	DX11::TextureShader * ParseTextureShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser);

public:
	ShaderLoader(Api * gpu, Files::Directory * directory, const wchar_t * path);

	void ParseXml() override
	{
		if(!parser) { failed = true; return; }
		tinyxml2::XMLElement * targetApiElement = parser->GetApiElement("directx11");
		if(!targetApiElement) { failed = true; return; }
		if(parser->IsModelShader())
		{
			asset = ParseModelShaderXML(targetApiElement, parser);
		}
		else
		{
			asset = ParseTextureShaderXML(targetApiElement, parser);
		}
		if(!asset) failed = true;
	}
	bool HasParsedXml() override
	{
		return (vertexShadersLoaded == numVertexShaders
			&& pixelShadersLoaded == numPixelShaders) || failed;
	}
};

} // namespace DX11
} // namespace Ingenuity

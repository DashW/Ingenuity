#pragma once

#include <vector>
#include "GpuShaders.h"
#include "StepMgr.h"
#include "FilesApi.h"

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

namespace Ingenuity {
namespace Gpu {

class ShaderParser
{
	tinyxml2::XMLDocument * document;

	std::vector<Gpu::ShaderParamSpec> paramSpecs;
	std::vector<Gpu::SamplerParam> samplerParams;

	std::string shaderName;

	bool isModelShader;
	bool isComputeShader;

	void ParseParams(tinyxml2::XMLElement * element);
	void ParseSamplerParams(unsigned paramIndex, tinyxml2::XMLElement * element);
	Gpu::SamplerParam::AddressMode ParseAddressMode(const char * text);

public:
	ShaderParser(char * buffer, unsigned bufferSize);
	virtual ~ShaderParser();

	tinyxml2::XMLElement * GetApiElement(const char * targetApi);

	bool IsModelShader() { return isModelShader; }
	bool IsComputeShader() { return isComputeShader; }
	bool IsTextureShader() { return !isModelShader && !isComputeShader; }
	unsigned GetNumParams() { return paramSpecs.size(); }
	Gpu::ShaderParamSpec & GetParamSpec(unsigned index) { return paramSpecs[index]; }
	unsigned GetNumSamplerParams() { return samplerParams.size(); }
	Gpu::SamplerParam & GetSamplerParam(unsigned index) { return samplerParams[index]; }
	std::string GetShaderName() { return shaderName; }
};

struct ShaderLoader : public SimpleLoader
{
protected:
	Files::File * file;
	Files::Api * files;
	ShaderParser * parser;

	virtual void ParseXml() = 0;
	virtual bool IsFinished() = 0;

	volatile bool loaderFailed;

public:
	ShaderLoader(Files::Api * files, Files::Directory * directory, const wchar_t * path) :
		SimpleLoader(files, directory, path, ShaderAsset), files(files), parser(0), loaderFailed(false) {}
	virtual ~ShaderLoader()
	{
		if(parser) delete parser;
	}

	virtual void Respond() override
	{
		if(buffer)
		{
			parser = new ShaderParser(buffer, bufferLength);
			ParseXml();
		}
		else
		{
			loaderFailed = true;
		}
	}

	virtual bool IsAssetReady() override { return complete && (IsFinished() || loaderFailed); }
};

} // namespace Gpu
} // namespace Ingenuity

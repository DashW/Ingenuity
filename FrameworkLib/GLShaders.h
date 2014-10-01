#pragma once

#ifdef USE_GL_GPUAPI

#include "GLApi.h"
#include "ShaderParser.h"

namespace Ingenuity {
namespace GL {

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
		//ShaderStage shader;
		std::string paramName;
		int uniformLocation = -1;
	};

	typedef std::map<unsigned, ParamMapping> ParamMap;

	Shader(bool modelShader) : Gpu::Shader(modelShader) {}
	virtual ~Shader() {}

	static bool ValidateProgram(unsigned program);

	virtual bool OnShaderCompiled() = 0;

	//static void ApplyTextureParameter(unsigned registerIndex, Gpu::ShaderParam * param);

	//static void UpdateConstantBuffer(std::vector<float> & constants, ID3D11Buffer ** buffer);
};

struct ModelShader : public Shader
{
	//static const unsigned NUM_STANDARD_BUFFERS = 2;
	//static const unsigned NUM_PARAM_BUFFERS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - NUM_STANDARD_BUFFERS;
	static const unsigned MAX_LIGHTS = 6;

	struct Technique
	{
		//ID3D11InputLayout * inputLayout;
		unsigned vertexShader;
		unsigned pixelShader; 
		unsigned shaderProgram;

		ParamMap paramMappings;

		int projMatrixLocation;
		int viewMatrixLocation;
		int modelMatrixLocation;

		Technique() : 
			vertexShader(0), 
			pixelShader(0), 
			shaderProgram(0), 
			projMatrixLocation(-1), 
			viewMatrixLocation(-1),
			modelMatrixLocation(-1)
		{}
		~Technique() {}

		//bool SetExtraParameters(Gpu::Effect * effect);

		//void ApplySamplerParams(std::vector<Gpu::SamplerParam> & samplerParams);
	};

	std::map<unsigned, Technique> techniques;
	Technique * currentTechnique;

	ModelShader() : Shader(true), currentTechnique(0) {}
	virtual ~ModelShader() {}

	virtual bool OnShaderCompiled() override;

	bool SetTechnique(VertexType vType, InstanceType iType);
	bool SetParameters(Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights, unsigned numLights, float aspect);
};

struct TextureShader : public Shader
{
	ParamMap paramMappings;

	static unsigned vertexShader;
	unsigned pixelShader;
	unsigned shaderProgram;

	TextureShader() : Shader(false), pixelShader(0), shaderProgram(0) {}
	virtual ~TextureShader() {}

	virtual bool OnShaderCompiled() override;

	//virtual bool SetParameters(Gpu::Texture * texture, Gpu::Effect * effect);

//protected:
	//bool SetExtraParameters(Gpu::Effect * effect);
};

class ShaderLoader : public Gpu::ShaderLoader
{
	GL::Api * gpu;
	unsigned numVertexShaders;
	unsigned numPixelShaders;
	unsigned vertexShadersLoaded;
	unsigned pixelShadersLoaded;

	struct ShaderResponse;

	static bool ValidateShader(unsigned shader);
	bool ParseParamMappingsXML(tinyxml2::XMLElement * element, GL::Shader * shader, GL::Shader::ParamMap & paramMappings);
	GL::ModelShader * ParseModelShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser);
	GL::TextureShader * ParseTextureShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser);

	GL::Shader * GetShader() { return static_cast<GL::Shader*>(asset); }

public:
	ShaderLoader(GL::Api * gpu, Files::Api * files, Files::Directory * directory, const wchar_t * path);
	virtual ~ShaderLoader() {}

	void ParseXml() override
	{
		if(!parser) { failed = true; return; }
		tinyxml2::XMLElement * targetApiElement = parser->GetApiElement("opengl");
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


} // namespace GL
} // namespace Ingenuity

#endif // USE_GL_GPUAPI

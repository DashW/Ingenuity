#include "DX11ShaderLoader.h"
#include "DX11Api.h"

namespace Ingenuity {

DX11::ShaderLoader::ShaderLoader(DX11::Api * gpu, Files::Directory * directory, const wchar_t * path) :
	Gpu::ShaderLoader(gpu->files, directory, path),
	gpu(gpu),
	numVertexShaders(0),
	numPixelShaders(0),
	vertexShadersLoaded(0),
	pixelShadersLoaded(0),
	failed(false)
{
}

bool DX11::ShaderLoader::ParseParamMappingsXML(
	tinyxml2::XMLElement * element, 
	DX11::Shader * shader, 
	DX11::Shader::ParamMap & paramMappings)
{
	tinyxml2::XMLElement * mappingElement = element->FirstChildElement("paramMapping");
	while(mappingElement)
	{
		DX11::Shader::ParamMapping mapping;
		unsigned paramIndex = mappingElement->UnsignedAttribute("index");

		if(paramIndex >= shader->paramSpecs.size())
		{
			OutputDebugString(L"Model shader param mapping has an invalid index!\n");
			return false;
		}

		mapping.shader = DX11::Shader::Pixel;
		const char * shaderStageChars = mappingElement->Attribute("shader");
		if(shaderStageChars)
		{
			if(strcmp(shaderStageChars, "vertex") == 0)
			{
				mapping.shader = DX11::Shader::Vertex;
			}
			if(strcmp(shaderStageChars, "geometry") == 0)
			{
				OutputDebugString(L"Not yet implemented!\n");
			}
		}

		Gpu::ShaderParam::Type type = shader->paramSpecs[paramIndex].type;
		if(type == Gpu::ShaderParam::TypeFloat || type == Gpu::ShaderParam::TypeFloatArray)
		{
			mapping.registerIndex = mappingElement->UnsignedAttribute("bufferIndex");
			mapping.bufferOffset = mappingElement->UnsignedAttribute("bufferOffset");
		}
		else // Texture
		{
			if(mappingElement->QueryUnsignedAttribute("textureIndex", &(mapping.registerIndex)) != tinyxml2::XML_NO_ERROR)
			{
				OutputDebugString(L"Texture parameter has no valid textureIndex attribute!\n");
				return false;
			}
		}

		paramMappings[paramIndex] = mapping;

		mappingElement = mappingElement->NextSiblingElement("paramMapping");
	}

	return true;
}

struct DX11::ShaderLoader::VertexShaderResponse : public Files::Response
{
	ShaderLoader * loader;
	ID3D11InputLayout ** inputLayout;
	ID3D11VertexShader ** vertexObject;
	VertexType vertexType;
	InstanceType instanceType;

	VertexShaderResponse(ShaderLoader * l, ID3D11InputLayout ** il, ID3D11VertexShader ** s, VertexType v, InstanceType i) :
		loader(l), inputLayout(il), vertexObject(s), vertexType(v), instanceType(i) {}

	virtual void Respond() override
	{
		closeOnComplete = true; deleteOnComplete = true;

		if(buffer)
		{
			unsigned techniqueKey = VertApi::GetTechniqueKey(vertexType, instanceType);

			if(!(*inputLayout))
			{
				std::vector<D3D11_INPUT_ELEMENT_DESC> elementDescs;
				const D3D11_INPUT_ELEMENT_DESC * desc = loader->gpu->vertexDescs[vertexType];
				unsigned size = loader->gpu->vertexDescSizes[vertexType];
				const D3D11_INPUT_ELEMENT_DESC * iDesc = loader->gpu->instanceDescs[instanceType];
				unsigned iSize = loader->gpu->instanceDescSizes[instanceType];

				for(unsigned i = 0; i < size; ++i)
				{
					elementDescs.push_back(desc[i]);
				}
				for(unsigned i = 0; i < iSize; ++i)
				{
					elementDescs.push_back(iDesc[i]);
				}

				if(elementDescs.size() > 0)
				{
					loader->gpu->direct3Ddevice->CreateInputLayout(
						elementDescs.data(),
						elementDescs.size(),
						buffer,
						bufferLength,
						inputLayout);
				}
			}
			loader->gpu->direct3Ddevice->CreateVertexShader(buffer, bufferLength, 0, vertexObject);

			if(*vertexObject)
			{
				loader->vertexShadersLoaded++;
			}
			else
			{
				loader->failed = true;
			}
		}
	}
};

struct DX11::ShaderLoader::PixelShaderResponse : public Files::Response
{
	DX11::ShaderLoader * loader;
	ID3D11PixelShader ** pixelObject;

	PixelShaderResponse(DX11::ShaderLoader * l, ID3D11PixelShader ** p) :
		loader(l), pixelObject(p) {}

	virtual void Respond() override
	{
		closeOnComplete = true; deleteOnComplete = true;

		if(buffer)
		{
			loader->gpu->direct3Ddevice->CreatePixelShader(buffer, bufferLength, 0, pixelObject);
			if(*pixelObject)
			{
				loader->pixelShadersLoaded++;
			}
			else
			{
				loader->failed = true;
			}
		}
		else
		{
			// Instead of failing the entire loader, could we not just fail the technique?
			loader->failed = true;
		}
	}
};

DX11::ModelShader * DX11::ShaderLoader::ParseModelShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser)
{
	DX11::ModelShader * shader = new DX11::ModelShader(gpu->direct3Ddevice);

	for(unsigned i = 0; i < parser->GetNumParams(); ++i)
	{
		shader->paramSpecs.push_back(parser->GetParamSpec(i));
	}
	for(unsigned i = 0; i < parser->GetNumSamplerParams(); ++i)
	{
		shader->defaultSamplerParams.push_back(parser->GetSamplerParam(i));
	}

	tinyxml2::XMLElement * techniqueElement = element->FirstChildElement("technique");
	while(techniqueElement)
	{
		VertexType vertexType = VertexType_Pos;

		const char * vertexTypeChars = techniqueElement->Attribute("vertexType");
		if(!vertexTypeChars)
		{
			OutputDebugString(L"Model shader technique has no vertex type!\n");
			delete shader;
			return 0;
		}

		for(unsigned i = 0; i < VertexType_Count; ++i)
		{
			if(strcmp(vertexTypeChars, VertApi::GetVertexName((VertexType)i)) == 0)
			{
				vertexType = (VertexType)i;
				break;
			}
		}

		InstanceType instanceType = InstanceType_None;

		const char * instanceTypeChars = techniqueElement->Attribute("instanceType");
		if(instanceTypeChars)
		{
			if(strcmp(instanceTypeChars, "Pos") == 0)
			{
				instanceType = InstanceType_Pos;
			}
			if(strcmp(instanceTypeChars, "PosCol") == 0)
			{
				instanceType = InstanceType_PosCol;
			}
		}

		unsigned key = VertApi::GetTechniqueKey(vertexType, instanceType);
		shader->techniques[key] = DX11::ModelShader::Technique();
		DX11::ModelShader::Technique & technique = shader->techniques[key];

		const char * vertexShaderName = techniqueElement->Attribute("vertexShader");
		const char * pixelShaderName = techniqueElement->Attribute("pixelShader");

		if(!vertexShaderName || !pixelShaderName)
		{
			OutputDebugString(L"Model shader technique is missing vertex or pixel shader attributes!\n");
			delete shader;
			return 0;
		}

		if(!ParseParamMappingsXML(techniqueElement, shader, technique.paramMappings))
		{
			delete shader;
			return 0;
		}

		std::string vertexShaderFilename(vertexShaderName);
		std::string pixelShaderFilename(pixelShaderName);
		vertexShaderFilename += ".cso";
		pixelShaderFilename += ".cso";

		std::wstring vertexShaderPath(vertexShaderFilename.begin(), vertexShaderFilename.end());
		std::wstring pixelShaderPath(pixelShaderFilename.begin(), pixelShaderFilename.end());

		Files::Directory * rootDir = files->GetKnownDirectory(Files::FrameworkDir);

		files->OpenAndRead(rootDir, vertexShaderPath.c_str(),
			new VertexShaderResponse(this, &(technique.inputLayout), &(technique.vertexObject), vertexType, instanceType));

		files->OpenAndRead(rootDir, pixelShaderPath.c_str(),
			new PixelShaderResponse(this, &(technique.pixelObject)));

		numVertexShaders++;
		numPixelShaders++;

		techniqueElement = techniqueElement->NextSiblingElement("technique");
	}

	return shader;
}

DX11::TextureShader * DX11::ShaderLoader::ParseTextureShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser)
{
	DX11::TextureShader * shader = new DX11::TextureShader(gpu->direct3Ddevice);
	Files::Directory * rootDir = files->GetKnownDirectory(Files::FrameworkDir);

	for(unsigned i = 0; i < parser->GetNumParams(); ++i)
	{
		shader->paramSpecs.push_back(parser->GetParamSpec(i));
	}
	for(unsigned i = 0; i < parser->GetNumSamplerParams(); ++i)
	{
		shader->defaultSamplerParams.push_back(parser->GetSamplerParam(i));
	}

	if(!gpu->texVertexShaderRequested)
	{
		gpu->texVertexShaderRequested = true;

		files->OpenAndRead(rootDir, L"TextureCopyVtxPosTex.cso",
			new VertexShaderResponse(this, &(gpu->texShaderLayout), &(gpu->texVertexShader), VertexType_PosTex, InstanceType_None));

		numVertexShaders++;
	}

	const char * pixelShaderName = element->Attribute("shader");
	if(!pixelShaderName || !ParseParamMappingsXML(element, shader, shader->paramMappings))
	{
		delete shader;
		return 0;
	}

	std::string pixelShaderFilename(pixelShaderName);
	pixelShaderFilename += ".cso";
	std::wstring pixelShaderPath(pixelShaderFilename.begin(), pixelShaderFilename.end());

	files->OpenAndRead(rootDir, pixelShaderPath.c_str(),
		new PixelShaderResponse(this, &(shader->pixelObject)));

	numPixelShaders++;

	return shader;
}

} // namespace Ingenuity

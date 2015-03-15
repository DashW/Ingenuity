#ifdef USE_DX11_GPUAPI

#include "DX11ShaderLoader.h"
#include "DX11Api.h"

namespace Ingenuity {

const char* DX11::ShaderLoader::primitiveStrings[Shader::PrimitiveCount] = {
	"",
	"point",
	"line",
	"triangle"
};

DX11::ShaderLoader::ShaderLoader(DX11::Api * gpu, Files::Api * files, Files::Directory * directory, const wchar_t * path) 
	: Gpu::ShaderLoader(files, directory, path)
	, gpu(gpu)
	, pendingComputeResponse(0)
{
}

DX11::ShaderLoader::~ShaderLoader()
{
	DeleteResponses();
}

bool DX11::ShaderLoader::ParseParamMappingsXML(
	tinyxml2::XMLElement * element, 
	DX11::Shader * shader, 
	DX11::Shader::ParamMap & paramMappings)
{
	tinyxml2::XMLElement * mappingElement = element->FirstChildElement("paramMapping");
	while(mappingElement)
	{
		DX11::Shader::ParamMapping mapping = { 0, DX11::Shader::Pixel, 0 };

		if(mappingElement->QueryUnsignedAttribute("index",&mapping.paramIndex) != tinyxml2::XML_NO_ERROR ||
			mapping.paramIndex >= shader->paramSpecs.size())
		{
			OutputDebugString(L"Model shader param mapping has an undefined or invalid index!\n");
			return false;
		}

		mapping.shader = parser->IsComputeShader() ? DX11::Shader::Compute : DX11::Shader::Pixel;
		const char * shaderStageChars = mappingElement->Attribute("shader");
		if(shaderStageChars)
		{
			if(strcmp(shaderStageChars, "vertex") == 0)
			{
				mapping.shader = DX11::Shader::Vertex;
			}
			if(strcmp(shaderStageChars, "geometry") == 0)
			{
				mapping.shader = DX11::Shader::Geometry;
			}
		}

		Gpu::ShaderParam::Type type = shader->paramSpecs[mapping.paramIndex].type;
		if(type == Gpu::ShaderParam::TypeInteger || 
			type == Gpu::ShaderParam::TypeUnsigned ||
			type == Gpu::ShaderParam::TypeFloat )
		{
			mapping.registerIndex = mappingElement->UnsignedAttribute("bufferIndex");
			mapping.bufferOffset = mappingElement->UnsignedAttribute("bufferOffset");
		}
		else if(type == Gpu::ShaderParam::TypeFloatArray)
		{
			if(mappingElement->QueryUnsignedAttribute("bufferIndex", &(mapping.registerIndex)) == tinyxml2::XML_NO_ERROR)
			{
				mapping.bufferOffset = mappingElement->UnsignedAttribute("bufferOffset");
			}
			else
			{
				OutputDebugString(L"FloatArray parameter has no valid bufferIndex attribute!\n");
				return false;
			}
		}
		else if(type == Gpu::ShaderParam::TypeParamBuffer)
		{
			mapping.registerIndex = mappingElement->UnsignedAttribute("bufferIndex");
			mapping.writeable = mappingElement->BoolAttribute("writeable");
			const char * attributeChars = mappingElement->Attribute("attribute");
			if(attributeChars)
			{
				if(strcmp(attributeChars, "size") == 0)
				{
					mapping.attribute = Shader::BufferAttribute::Size;
				}
			}
		}
		else // Texture
		{
			if(mappingElement->QueryUnsignedAttribute("textureIndex", &(mapping.registerIndex)) != tinyxml2::XML_NO_ERROR)
			{
				OutputDebugString(L"Texture parameter has no valid textureIndex attribute!\n");
				return false;
			}
		}

		paramMappings.push_back(mapping);

		mappingElement = mappingElement->NextSiblingElement("paramMapping");
	}

	return true;
}

struct DX11::ShaderLoader::VertexShaderResponse : public Files::Response
{
	DX11::Api * gpu;
	ID3D11InputLayout ** inputLayout;
	ID3D11VertexShader ** shaderObject;
	VertexType vertexType;
	InstanceType instanceType;
	ShaderLoadState loadState;

	VertexShaderResponse(DX11::Api * gpu, ID3D11InputLayout ** il, ID3D11VertexShader ** s, VertexType v, InstanceType i) :
		gpu(gpu), inputLayout(il), shaderObject(s), vertexType(v), instanceType(i) {}

	virtual void Respond() override
	{
		closeOnComplete = true;

		if(buffer)
		{
			unsigned techniqueKey = VertApi::GetTechniqueKey(vertexType, instanceType);

			if(inputLayout && !(*inputLayout))
			{
				std::vector<D3D11_INPUT_ELEMENT_DESC> elementDescs;
				const D3D11_INPUT_ELEMENT_DESC * desc = gpu->vertexDescs[vertexType];
				unsigned size = gpu->vertexDescSizes[vertexType];
				const D3D11_INPUT_ELEMENT_DESC * iDesc = gpu->instanceDescs[instanceType];
				unsigned iSize = gpu->instanceDescSizes[instanceType];

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
					gpu->direct3Ddevice->CreateInputLayout(
						elementDescs.data(),
						elementDescs.size(),
						buffer,
						bufferLength,
						inputLayout);
				}
			}
			gpu->direct3Ddevice->CreateVertexShader(buffer, bufferLength, 0, shaderObject);

			if(*shaderObject)
			{
				loadState = SUCCEEDED;
				return;
			}
		}
		loadState = FAILED;
	}
};

struct DX11::ShaderLoader::GeometryShaderResponse : public Files::Response
{
	DX11::Api * gpu;
	ID3D11GeometryShader ** shaderObject;
	ShaderLoadState loadState;

	GeometryShaderResponse(DX11::Api * gpu, ID3D11GeometryShader ** shader) 
		: gpu(gpu)
		, shaderObject(shader) 
	{}

	virtual void Respond() override
	{
		closeOnComplete = true;

		if(buffer)
		{
			gpu->direct3Ddevice->CreateGeometryShader(buffer, bufferLength, 0, shaderObject);
			if(*shaderObject)
			{
				loadState = SUCCEEDED;
				return;
			}
		}
		loadState = FAILED;
	}
};

struct DX11::ShaderLoader::PixelShaderResponse : public Files::Response
{
	DX11::Api * gpu;
	ID3D11PixelShader ** shaderObject;
	ShaderLoadState loadState;

	PixelShaderResponse(DX11::Api * gpu, ID3D11PixelShader ** shader) 
		: gpu(gpu)
		, shaderObject(shader) 
	{}

	virtual void Respond() override
	{
		closeOnComplete = true;

		if(buffer)
		{
			gpu->direct3Ddevice->CreatePixelShader(buffer, bufferLength, 0, shaderObject);
			if(*shaderObject)
			{
				loadState = SUCCEEDED;
				return;
			}
		}
		loadState = FAILED;
	}
};

struct DX11::ShaderLoader::ComputeShaderResponse : public Files::Response
{
	DX11::Api * gpu;
	ID3D11ComputeShader ** shaderObject;
	ShaderLoadState loadState;

	ComputeShaderResponse(DX11::Api * gpu, ID3D11ComputeShader ** shader)
		: gpu(gpu)
		, shaderObject(shader)
	{}

	virtual void Respond() override
	{
		closeOnComplete = true;

		if(buffer)
		{
			gpu->direct3Ddevice->CreateComputeShader(buffer, bufferLength, 0, shaderObject);
			if(*shaderObject)
			{
				loadState = SUCCEEDED;
				return;
			}
		}
		loadState = FAILED;
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
		bool indirect = techniqueElement->BoolAttribute("indirect");
		VertexType vertexType = VertexType_Pos;
		InstanceType instanceType = InstanceType_None;
		DX11::ModelShader::Technique * technique;

		if(indirect)
		{
			shader->CreateIndirectBuffer(gpu->direct3Ddevice);

			const char * primitiveChars = techniqueElement->Attribute("primitive");
			for(unsigned i = 0; i < Shader::PrimitiveCount; ++i)
			{
				if(strcmp(primitiveChars, primitiveStrings[i]) == 0)
				{
					shader->indirectPrimitive = (Shader::IndirectPrimitive)i;
					break;
				}
			}

			technique = &shader->indirectTechnique;
			pendingTechniques.emplace_back(0);
		}
		else
		{

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

			const char * instanceTypeChars = techniqueElement->Attribute("instanceType");
			if(instanceTypeChars)
			{
				for(unsigned i = 0; i < InstanceType_Count; ++i)
				{
					if(strcmp(instanceTypeChars, VertApi::GetInstanceName((InstanceType)i)) == 0)
					{
						instanceType = (InstanceType)i;
						break;
					}
				}
			}

			unsigned key = VertApi::GetTechniqueKey(vertexType, instanceType);
			pendingTechniques.emplace_back(key);
			shader->vertexTechniques[key] = DX11::ModelShader::Technique();
			technique = &shader->vertexTechniques[key];
		}

		const char * vertexShaderName = techniqueElement->Attribute("vertexShader");
		const char * geometryShaderName = techniqueElement->Attribute("geometryShader");
		const char * pixelShaderName = techniqueElement->Attribute("pixelShader");

		if(!vertexShaderName || !pixelShaderName)
		{
			OutputDebugString(L"Model shader technique is missing vertex or pixel shader attributes!\n");
			delete shader;
			return 0;
		}

		if(!ParseParamMappingsXML(techniqueElement, shader, technique->paramMappings))
		{
			delete shader;
			return 0;
		}

		Files::Directory * rootDir = files->GetKnownDirectory(Files::FrameworkDir);
		PendingTechnique & pendingTechnique = pendingTechniques.back();

		std::string vertexShaderFilename(vertexShaderName);
		vertexShaderFilename += ".cso";
		std::wstring vertexShaderPath(vertexShaderFilename.begin(), vertexShaderFilename.end());
		pendingTechnique.vertexResponse = new VertexShaderResponse(gpu, indirect ? 0 : &(technique->inputLayout), &(technique->vertexObject), vertexType, instanceType);
		files->OpenAndRead(rootDir, vertexShaderPath.c_str(), pendingTechnique.vertexResponse);

		if(geometryShaderName)
		{
			std::string geometryShaderFilename(geometryShaderName);
			geometryShaderFilename += ".cso";
			std::wstring geometryShaderPath(geometryShaderFilename.begin(), geometryShaderFilename.end());
			pendingTechnique.geometryResponse = new GeometryShaderResponse(gpu, &(technique->geometryObject));
			files->OpenAndRead(rootDir, geometryShaderPath.c_str(), pendingTechnique.geometryResponse);
		}

		std::string pixelShaderFilename(pixelShaderName);
		pixelShaderFilename += ".cso";
		std::wstring pixelShaderPath(pixelShaderFilename.begin(), pixelShaderFilename.end());
		pendingTechnique.pixelResponse = new PixelShaderResponse(gpu, &(technique->pixelObject));
		files->OpenAndRead(rootDir, pixelShaderPath.c_str(), pendingTechnique.pixelResponse);


		techniqueElement = techniqueElement->NextSiblingElement("technique");
	}

	return shader;
}

DX11::TextureShader * DX11::ShaderLoader::ParseTextureShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser)
{
	DX11::TextureShader * shader = new DX11::TextureShader(gpu->direct3Ddevice);
	Files::Directory * rootDir = files->GetKnownDirectory(Files::FrameworkDir);

	pendingTechniques.emplace_back(0);
	PendingTechnique & pendingTechnique = pendingTechniques.back();

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

		VertexShaderResponse * texVertexShaderResponse =
			new VertexShaderResponse(gpu, &(gpu->texShaderLayout), &(gpu->texVertexShader), VertexType_PosTex, InstanceType_None);

		texVertexShaderResponse->deleteOnComplete = true;

		files->OpenAndRead(rootDir, L"TextureCopyVtxPosTex.cso", texVertexShaderResponse);
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

	pendingTechnique.pixelResponse = new PixelShaderResponse(gpu, &(shader->pixelObject));

	files->OpenAndRead(rootDir, pixelShaderPath.c_str(), pendingTechnique.pixelResponse);

	return shader;
}

DX11::ComputeShader * DX11::ShaderLoader::ParseComputeShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser)
{
	DX11::ComputeShader * shader = new DX11::ComputeShader(gpu->direct3Ddevice);
	Files::Directory * rootDir = files->GetKnownDirectory(Files::FrameworkDir);

	for(unsigned i = 0; i < parser->GetNumParams(); ++i)
	{
		shader->paramSpecs.push_back(parser->GetParamSpec(i));
	}
	for(unsigned i = 0; i < parser->GetNumSamplerParams(); ++i)
	{
		shader->defaultSamplerParams.push_back(parser->GetSamplerParam(i));
	}

	const char * computeShaderName = element->Attribute("shader");
	if(!computeShaderName || !ParseParamMappingsXML(element, shader, shader->paramMappings))
	{
		delete shader;
		return 0;
	}

	std::string pixelShaderFilename(computeShaderName);
	pixelShaderFilename += ".cso";
	std::wstring pixelShaderPath(pixelShaderFilename.begin(), pixelShaderFilename.end());

	pendingComputeResponse = new ComputeShaderResponse(gpu, &(shader->computeObject));

	files->OpenAndRead(rootDir, pixelShaderPath.c_str(), pendingComputeResponse);

	return shader;
}

void DX11::ShaderLoader::DeleteResponses()
{
	for(unsigned i = 0; i < pendingTechniques.size(); ++i)
	{
		if(pendingTechniques[i].vertexResponse && pendingTechniques[i].vertexResponse->loadState != LOADING) 
			delete pendingTechniques[i].vertexResponse;
		if(pendingTechniques[i].geometryResponse && pendingTechniques[i].geometryResponse->loadState != LOADING) 
			delete pendingTechniques[i].geometryResponse;
		if(pendingTechniques[i].pixelResponse && pendingTechniques[i].pixelResponse->loadState != LOADING) 
			delete pendingTechniques[i].pixelResponse;
	}
	if(pendingComputeResponse && pendingComputeResponse->loadState != LOADING) 
		delete pendingComputeResponse;
}

bool DX11::ShaderLoader::IsFinished()
{
	// If ALL shaders in ALL techniques have finished loading
	// and ANY shader in ANY technique has failed to load
	// then remove that technique. 

	if(parser->IsComputeShader())
	{
		if(pendingComputeResponse)
		{
			ShaderLoadState loadState = pendingComputeResponse->loadState;
			if(loadState == LOADING) return false;
			if(loadState == FAILED) loaderFailed = true;
		}
	}
	else
	{
		for(unsigned i = 0; i < pendingTechniques.size(); ++i)
		{
			bool techniqueLoading = false;
			bool techniqueFailed = false;

			if(pendingTechniques[i].vertexResponse)
			{
				ShaderLoadState loadState = pendingTechniques[i].vertexResponse->loadState;
				if(loadState == LOADING) techniqueLoading = true;
				if(loadState == FAILED) techniqueFailed = true;
			}
			if(pendingTechniques[i].geometryResponse)
			{
				ShaderLoadState loadState = pendingTechniques[i].geometryResponse->loadState;
				if(loadState == LOADING) techniqueLoading = true;
				if(loadState == FAILED) techniqueFailed = true;
			}
			if(pendingTechniques[i].pixelResponse)
			{
				ShaderLoadState loadState = pendingTechniques[i].pixelResponse->loadState;
				if(loadState == LOADING) techniqueLoading = true;
				if(loadState == FAILED) techniqueFailed = true;
			}

			if(techniqueLoading)
			{
				return false;
			}
			else if(techniqueFailed)
			{
				if(parser->IsModelShader())
				{
					OutputDebugString(L"Failed to load shader technique!");

					DX11::ModelShader * shader = static_cast<DX11::ModelShader*>(asset);
					shader->vertexTechniques.erase(pendingTechniques[i].key);
					if(shader->vertexTechniques.size() < 1)
					{
						loaderFailed = true;
					}
				}
				else
				{
					loaderFailed = true;
				}
			}
		}
	}

	//DeleteResponses();
	return true;
}

} // namespace Ingenuity

#endif // USE_DX11_GPUAPI

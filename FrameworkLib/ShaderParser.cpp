#include "ShaderParser.h"

#include "GpuApi.h"
#include "GpuVertices.h"
#include "tinyxml2.h"

namespace Ingenuity {

static const char * paramTypeStrings[Gpu::ShaderParam::TypeCount] = 
{
	"integer",
	"unsigned",
	"float",
	"floatArray",
	"tex2D",
	"texCube",
	"tex3D",
	"drawSurface",
	"paramBuffer"
};

Gpu::ShaderParser::ShaderParser(char * buffer, unsigned bufferSize)
	: isModelShader(true), isComputeShader(false)
{
	document = new tinyxml2::XMLDocument();

	if(document->Parse(buffer, bufferSize) != tinyxml2::XML_NO_ERROR)
	{
		delete document; document = 0;
	}
}

Gpu::ShaderParser::~ShaderParser()
{
	if(document) delete document;
}

void Gpu::ShaderParser::ParseParams(tinyxml2::XMLElement * element)
{
	ParseSamplerParams(SamplerParam::DEFAULT_SAMPLER, element);

	tinyxml2::XMLElement * paramElement = element->FirstChildElement("param");
	while(paramElement)
	{
		ShaderParamSpec paramSpec;
		unsigned index = paramElement->UnsignedAttribute("index");
		const char * type = paramElement->Attribute("type");

		const char * displayName = paramElement->Attribute("displayName");
		if(displayName) paramSpec.displayName = displayName;

		if(type)
		{
			for(unsigned i = 0; i < ShaderParam::TypeCount; ++i)
			{
				if(strcmp(type, paramTypeStrings[i]) == 0)
				{
					paramSpec.type = (ShaderParam::Type) i;
					break;
				}
			}

			if(paramSpec.type == ShaderParam::TypeFloat)
			{
				paramSpec.defaultFValue = paramElement->FloatAttribute("default");
				if(paramElement->Attribute("min") || paramElement->Attribute("max"))
				{
					paramSpec.ranged = true;
					if(paramElement->QueryAttribute("min", &paramSpec.rangeMin) != tinyxml2::XML_NO_ERROR)
					{
						paramSpec.rangeMin = -FLT_MAX;
					}
					if(paramElement->QueryAttribute("max", &paramSpec.rangeMax) != tinyxml2::XML_NO_ERROR)
					{
						paramSpec.rangeMax = FLT_MAX;
					}
				}
			}

			// validate float array length ??

			if(paramSpec.type == ShaderParam::TypeTexture)
			{
				ParseSamplerParams(index, paramElement);
			}
		}

		while(paramSpecs.size() <= index)
		{
			paramSpecs.emplace_back();
		}
		paramSpecs[index] = paramSpec;

		paramElement = paramElement->NextSiblingElement("param");
	}
}

void Gpu::ShaderParser::ParseSamplerParams(unsigned paramIndex, tinyxml2::XMLElement * element)
{
	const char * addressUtext = element->Attribute("addressU");
	if(addressUtext)
	{
		samplerParams.emplace_back(paramIndex, Gpu::SamplerParam::AddressU, ParseAddressMode(addressUtext));
	}
	const char * addressVtext = element->Attribute("addressV");
	if(addressVtext)
	{
		samplerParams.emplace_back(paramIndex, Gpu::SamplerParam::AddressV, ParseAddressMode(addressVtext));
	}
	const char * addressWtext = element->Attribute("addressW");
	if(addressWtext)
	{
		samplerParams.emplace_back(paramIndex, Gpu::SamplerParam::AddressW, ParseAddressMode(addressWtext));
	}
	const char * filterModeText = element->Attribute("filterMode");
	if(filterModeText)
	{
		Gpu::SamplerParam::FilterMode filterMode = Gpu::SamplerParam::FilterGlobal;
		if(strcmp(filterModeText, "point") == 0)
		{
			filterMode = Gpu::SamplerParam::FilterPoint;
		}
		if(strcmp(filterModeText, "linear") == 0)
		{
			filterMode = Gpu::SamplerParam::FilterLinear;
		}
		if(strcmp(filterModeText, "anisotropic") == 0)
		{
			filterMode = Gpu::SamplerParam::FilterAnisotropic;
		}
		samplerParams.emplace_back(paramIndex, Gpu::SamplerParam::Filter, filterMode);
	}
	unsigned anisotropy = 0;
	if(element->QueryUnsignedAttribute("anisotropy", &anisotropy) == tinyxml2::XML_NO_ERROR)
	{
		samplerParams.emplace_back(paramIndex, Gpu::SamplerParam::Anisotropy, anisotropy);
	}
	const char * comparisonText = element->Attribute("comparison");
	if(comparisonText)
	{
		Gpu::SamplerParam::ComparisonFunc comparisonFunc = Gpu::SamplerParam::ComparisonNone;
		if(strcmp(comparisonText, "less") == 0)
		{
			comparisonFunc = Gpu::SamplerParam::ComparisonLess;
		}
		else if(strcmp(comparisonText, "greater") == 0)
		{
			comparisonFunc = Gpu::SamplerParam::ComparisonGreater;
		}
		else if(strcmp(comparisonText, "always") == 0)
		{
			comparisonFunc = Gpu::SamplerParam::ComparisonAlways;
		}
		else if(strcmp(comparisonText, "never") == 0)
		{
			comparisonFunc = Gpu::SamplerParam::ComparisonNever;
		}
		samplerParams.emplace_back(paramIndex, Gpu::SamplerParam::Comparison, comparisonFunc);
	}
}

Gpu::SamplerParam::AddressMode Gpu::ShaderParser::ParseAddressMode(const char * text)
{
	Gpu::SamplerParam::AddressMode addressMode = Gpu::SamplerParam::AddressWrap;
	if(strcmp(text, "mirror") == 0)
	{
		addressMode = Gpu::SamplerParam::AddressMirror;
	}
	if(strcmp(text, "clamp") == 0)
	{
		addressMode = Gpu::SamplerParam::AddressClamp;
	}
	return addressMode;
}

tinyxml2::XMLElement * Gpu::ShaderParser::GetApiElement(const char * targetApi)
{
	if(!document) return 0;

	tinyxml2::XMLElement * shaderRoot = document->FirstChildElement("shader");
	if(!shaderRoot) return 0;

	const char * shaderNameChars = shaderRoot->Attribute("name");
	const char * shaderTypeChars = shaderRoot->Attribute("type");

	if(!shaderNameChars || !shaderTypeChars) return 0;

	shaderName = shaderNameChars;
	isModelShader = strcmp(shaderTypeChars, "model") == 0;
	isComputeShader = strcmp(shaderTypeChars, "compute") == 0;

	tinyxml2::XMLElement * paramsRoot = shaderRoot->FirstChildElement("params");
	if(paramsRoot) ParseParams(paramsRoot);

	tinyxml2::XMLElement * targetApiElement = shaderRoot->FirstChildElement(targetApi);

	return targetApiElement;
}

} // namespace Ingenuity

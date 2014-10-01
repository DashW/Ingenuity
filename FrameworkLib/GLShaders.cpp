#ifdef USE_GL_GPUAPI

#include "GLShaders.h"

#include "tinyxml2.h"
#include <debugapi.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>

namespace Ingenuity {

unsigned GL::TextureShader::vertexShader = 0;

bool GL::Shader::ValidateProgram(unsigned program)
{
	const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	GLsizei length = 0;

	glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer); // Ask OpenGL to give us the log associated with the program
	//if(length > 0) // If we have any information to display
	//cout << "Program " << program << " link error: " << buffer << endl; // Output the information

	glValidateProgram(program); // Get OpenGL to try validating the program
	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status); // Find out if the shader program validated correctly
	//if(status == GL_FALSE) // If there was a problem validating
	//cout << "Error validating shader " << program << endl; // Output which program had the error

	return length == 0 && status == GL_TRUE;
}

bool GL::ModelShader::OnShaderCompiled()
{
	bool techniqueFailed = false;

	for(unsigned i = 0; i < techniques.size(); ++i)
	{
		Technique & technique = techniques[i];

		// If the both the vertex and fragment shaders have been compiled,
		// but no program has been created, create the program
		if(technique.vertexShader && technique.pixelShader && !technique.shaderProgram)
		{
			technique.shaderProgram = glCreateProgram(); // Create a GLSL program
			glAttachShader(technique.shaderProgram, technique.vertexShader); // Attach a vertex shader to the program
			glAttachShader(technique.shaderProgram, technique.pixelShader); // Attach the fragment shader to the program

			glBindAttribLocation(technique.shaderProgram, 0, "in_Position"); // Bind a constant attribute location for positions of vertices
			glBindAttribLocation(technique.shaderProgram, 1, "in_Color"); // Bind another constant attribute location, this time for color

			glLinkProgram(technique.shaderProgram); // Link the vertex and fragment shaders in the program
			
			// Validate the shader program
			if(!ValidateProgram(technique.shaderProgram))
			{
				techniqueFailed = true;
			}
			else
			{
				technique.projMatrixLocation = glGetUniformLocation(technique.shaderProgram, "projectionMatrix"); // Get the location of our projection matrix in the shader
				technique.viewMatrixLocation = glGetUniformLocation(technique.shaderProgram, "viewMatrix"); // Get the location of our view matrix in the shader
				technique.modelMatrixLocation = glGetUniformLocation(technique.shaderProgram, "modelMatrix"); // Get the location of our model matrix in the shader

				//if(technique.projMatrixLocation < 0 || technique.viewMatrixLocation < 0 || technique.modelMatrixLocation < 0)
				//{
				//	OutputDebugString(L"Matrix locations missing from shader!");
				//	techniqueFailed = true;
				//}
			}
		}
	}
	return !techniqueFailed;
}

bool GL::ModelShader::SetTechnique(VertexType vType, InstanceType iType)
{
	unsigned key = VertApi::GetTechniqueKey(vType, iType);

	std::map<unsigned, Technique>::iterator it = techniques.find(key);
	if(it == techniques.end())
	{
		OutputDebugString(L"Shader does not have technique for vertex/instance type!\n");
		return false;
	}

	currentTechnique = &(it->second);

	glUseProgram(currentTechnique->shaderProgram);

	return true;
}

bool GL::ModelShader::SetParameters(Gpu::Model * model, Gpu::Camera * camera, Gpu::Light ** lights, unsigned numLights, float aspect)
{
	if(!model || !camera || !currentTechnique) return false;
	if(camera->position == camera->target)
	{
		OutputDebugString(L"Cannot construct view matrix!");
		return false;
	}

	glm::mat4 modelMatrix = model->GetMatrix();
	glm::mat4 viewMatrix = camera->GetViewMatrix();
	glm::mat4 projMatrix = camera->GetProjMatrix(aspect);

	glUniformMatrix4fv(currentTechnique->projMatrixLocation, 1, GL_FALSE, &projMatrix[0][0]); // Send our projection matrix to the shader
	glUniformMatrix4fv(currentTechnique->viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]); // Send our view matrix to the shader
	glUniformMatrix4fv(currentTechnique->modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); // Send our model matrix to the shader

	return true;
}

bool GL::TextureShader::OnShaderCompiled()
{
	shaderProgram = glCreateProgram(); // Create a GLSL program
	glAttachShader(shaderProgram, vertexShader); // Attach a vertex shader to the program
	glAttachShader(shaderProgram, pixelShader); // Attach the fragment shader to the program

	glBindAttribLocation(shaderProgram, 0, "in_Position"); // Bind a constant attribute location for positions of vertices
	glBindAttribLocation(shaderProgram, 1, "in_Color"); // Bind another constant attribute location, this time for color

	glLinkProgram(shaderProgram); // Link the vertex and fragment shaders in the program

	// Validate the shader program
	return ValidateProgram(shaderProgram);
}

GL::ShaderLoader::ShaderLoader(GL::Api * gpu, Files::Api * files, Files::Directory * directory, const wchar_t * path) :
	Gpu::ShaderLoader(files, directory, path),
	gpu(gpu),
	numVertexShaders(0),
	numPixelShaders(0),
	vertexShadersLoaded(0),
	pixelShadersLoaded(0)
{
}

bool GL::ShaderLoader::ValidateShader(unsigned shader)
{
	const unsigned int BUFFER_SIZE = 512;
	char tempBuffer[BUFFER_SIZE];
	memset(tempBuffer, 0, BUFFER_SIZE);
	GLsizei length = 0;

	glGetShaderInfoLog(shader, BUFFER_SIZE, &length, tempBuffer); // Ask OpenGL to give us the log associated with the shader
	return length == 0;
}

bool GL::ShaderLoader::ParseParamMappingsXML(
	tinyxml2::XMLElement * element,
	GL::Shader * shader,
	GL::Shader::ParamMap & paramMappings)
{
	tinyxml2::XMLElement * mappingElement = element->FirstChildElement("paramMapping");
	while(mappingElement)
	{
		GL::Shader::ParamMapping mapping;
		unsigned paramIndex = mappingElement->UnsignedAttribute("index");

		if(paramIndex >= shader->paramSpecs.size())
		{
			OutputDebugString(L"Model shader param mapping has an invalid index!\n");
			return false;
		}

		//mapping.shader = GL::Shader::Pixel;
		//const char * shaderStageChars = mappingElement->Attribute("shader");
		//if(shaderStageChars)
		//{
		//	if(strcmp(shaderStageChars, "vertex") == 0)
		//	{
		//		mapping.shader = GL::Shader::Vertex;
		//	}
		//	if(strcmp(shaderStageChars, "geometry") == 0)
		//	{
		//		OutputDebugString(L"Not yet implemented!\n");
		//	}
		//}

		const char * paramNameChars = mappingElement->Attribute("name");
		if(paramNameChars)
		{
			mapping.paramName = paramNameChars;
		}

		paramMappings[paramIndex] = mapping;

		mappingElement = mappingElement->NextSiblingElement("paramMapping");
	}

	return true;
}

struct GL::ShaderLoader::ShaderResponse : public Files::Response
{
	ShaderLoader * loader;
	unsigned & shader;
	bool pixelShader;

	ShaderResponse(ShaderLoader * l, unsigned & s, bool p) :
		loader(l), shader(s), pixelShader(p) {}

	virtual void Respond() override
	{
		closeOnComplete = true; deleteOnComplete = true;

		if(buffer)
		{
			shader = glCreateShader(pixelShader ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);

			glShaderSource(shader, 1, &buffer, 0); // Set the source for the vertex shader to the loaded text
			glCompileShader(shader); // Compile the vertex shader

			if(ShaderLoader::ValidateShader(shader) && loader->GetShader()->OnShaderCompiled())
			{
				if(pixelShader)
				{
					loader->pixelShadersLoaded++;
				}
				else
				{
					loader->vertexShadersLoaded++;
				}
			}
			else
			{
				OutputDebugString(L"Failed to compile shader!\n");
				loader->failed = true;
			}
		}
		else
		{
			loader->failed = true;
		}
	}
};

GL::ModelShader * GL::ShaderLoader::ParseModelShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser)
{
	GL::ModelShader * shader = new GL::ModelShader();

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
			if(strcmp(instanceTypeChars, "PosSca") == 0)
			{
				instanceType = InstanceType_PosSca;
			}
		}

		unsigned key = VertApi::GetTechniqueKey(vertexType, instanceType);
		shader->techniques[key] = GL::ModelShader::Technique();
		GL::ModelShader::Technique & technique = shader->techniques[key];

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
		vertexShaderFilename += ".vert";
		pixelShaderFilename += ".frag";

		std::wstring vertexShaderPath(vertexShaderFilename.begin(), vertexShaderFilename.end());
		std::wstring pixelShaderPath(pixelShaderFilename.begin(), pixelShaderFilename.end());

		Files::Directory * rootDir = files->GetKnownDirectory(Files::FrameworkDir);

		files->OpenAndRead(rootDir, vertexShaderPath.c_str(),
			new ShaderResponse(this, technique.vertexShader, false));

		files->OpenAndRead(rootDir, pixelShaderPath.c_str(),
			new ShaderResponse(this, technique.pixelShader, true));

		numVertexShaders++;
		numPixelShaders++;

		techniqueElement = techniqueElement->NextSiblingElement("technique");
	}

	return shader;
}

GL::TextureShader * GL::ShaderLoader::ParseTextureShaderXML(tinyxml2::XMLElement * element, Gpu::ShaderParser * parser)
{
	GL::TextureShader * shader = new GL::TextureShader();
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

		files->OpenAndRead(rootDir, L"TextureCopyVtxPosTex.vert", new ShaderResponse(this, gpu->texVertexShader, false));

		numVertexShaders++;
	}

	const char * pixelShaderName = element->Attribute("shader");
	if(!pixelShaderName || !ParseParamMappingsXML(element, shader, shader->paramMappings))
	{
		delete shader;
		return 0;
	}

	std::string pixelShaderFilename(pixelShaderName);
	pixelShaderFilename += ".frag";
	std::wstring pixelShaderPath(pixelShaderFilename.begin(), pixelShaderFilename.end());

	files->OpenAndRead(rootDir, pixelShaderPath.c_str(),
		new ShaderResponse(this, shader->pixelShader, true));

	numPixelShaders++;

	return shader;
}

} // namespace Ingenuity

#endif // USE_GL_GPUAPI
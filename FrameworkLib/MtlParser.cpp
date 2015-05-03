#include "stdafx.h"
#include "MtlParser.h"
#include "AssetMgr.h"
#include "GpuApi.h"

#include <sstream>

namespace Ingenuity {

void MtlParser::ParseMtl(std::string text)
{
	std::stringstream stream(text);
	std::string line;

	while(!ready)
	{
		if(!std::getline(stream, line, '\n')) line = "";
		ParseLine(line.c_str());
	}
}

void MtlParser::ParseLine(std::string line)
{
	if(line.size() == 0)
	{
		if(textureTicket == -1)
		{
			// No textures to load
			ready = true;
		}
		else if(assets->IsLoaded(textureTicket))
		{
			for(unsigned i = 0; i < materials.size(); ++i)
			{
				materials[i].tex = assets->GetAsset<Gpu::Texture>(directory, materials[i].texPath.c_str());
				materials[i].normal = assets->GetAsset<Gpu::Texture>(directory, materials[i].normalPath.c_str());
			}
			ready = true;
		}
		return;
	}

	int pos = -1;
	if((pos = line.find("newmtl ")) > -1)
	{
		WavefrontMtl mtl;

		mtl.name = GetArguments(line, "newmtl ", pos);

		materials.emplace_back(mtl);
	}
	if((pos = line.find("map_Kd ")) > -1)
	{
		WavefrontMtl &mtl = materials[materials.size() - 1];

		std::string args = GetArguments(line, "map_Kd ", pos);
		std::wstring texPath = std::wstring(args.begin(), args.end());
		if(args.find(":") == -1)
		{
			texPath = subDirectory + texPath;
		}

		mtl.texPath = texPath;
		textureTicket = assets->Load(directory, texPath.c_str(), TextureAsset, 0, textureTicket);
	}
	if((pos = line.find("map_Bump ")) > -1)
	{
		WavefrontMtl &mtl = materials[materials.size() - 1];

		std::string args = GetArguments(line, "map_bump ", pos);
		std::wstring texPath = std::wstring(args.begin(), args.end());
		//unsigned spaceChar = wargs.find(L" ");
		//std::wstring texPath = wargs.substr(0,spaceChar < wargs.size() ? spaceChar : wargs.size());
		if(args.find(":") == -1)
		{
			texPath = subDirectory + texPath;
		}

		mtl.normalPath = texPath;
		textureTicket = assets->Load(directory, texPath.c_str(), TextureAsset, 0, textureTicket);
	}
	if((pos = line.find("bump ")) > -1)
	{
		WavefrontMtl &mtl = materials[materials.size() - 1];

		std::string args = GetArguments(line, "bump ", pos);
		std::wstring texPath = std::wstring(args.begin(), args.end());
		//unsigned spaceChar = wargs.find(L" ");
		//std::wstring texPath = wargs.substr(0, spaceChar < wargs.size() ? spaceChar : wargs.size());
		if(args.find(":") == -1)
		{
			texPath = subDirectory + texPath;
		}

		mtl.normalPath = texPath;
		textureTicket = assets->Load(directory, texPath.c_str(), TextureAsset, 0, textureTicket);
	}
	if((pos = line.find("Ns ")) > -1)
	{
		WavefrontMtl &mtl = materials[materials.size() - 1];
		std::string args = GetArguments(line, "Ns ", pos);

		// parse to a float;
	}
	if((pos = line.find("Kd ")) > -1)
	{
		WavefrontMtl &mtl = materials[materials.size() - 1];
		std::string args = GetArguments(line, "Kd ", pos);
		std::vector<std::string> values = Tokenize(args);

		if(values.size() >= 3)
		{
			mtl.color[0] = std::stof(values[0]);
			mtl.color[1] = std::stof(values[1]);
			mtl.color[2] = std::stof(values[2]);
		}
		if(values.size() >= 4)
		{
			mtl.color[3] = std::stof(values[3]);
		}
	}
}

std::string MtlParser::GetArguments(std::string line, const char * command, int pos)
{
	line = line.substr(pos + strlen(command));

	int comment = line.find_first_of("#");
	if(comment > -1) line.erase(comment);

	line.erase(line.find_last_not_of(" \n\r\t") + 1);

	return line;
}

// http://www.oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
std::vector<std::string> MtlParser::Tokenize(const std::string& str, const std::string& delimiters)
{
	std::vector<std::string> tokens;
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while(std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}

	return tokens;
}

WavefrontMtl* MtlParser::GetMtl(const char* name)
{
	for(unsigned i = 0; i < materials.size(); i++)
	{
		if(materials[i].name.compare(name) == 0)
		{
			return &materials[i];
		}
	}
	return 0;
}

} // namespace Ingenuity

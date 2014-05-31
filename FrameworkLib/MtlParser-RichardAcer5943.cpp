#include "MtlParser.h"
#include "GpuApi.h"

#include <sstream>

void MtlParser::ParseMtl(GpuApi *gpu, const char *text)
{
	std::stringstream stream(text);
	std::string line;

	while (std::getline(stream,line,'\n'))
	{
		int pos = -1;
		if ((pos = line.find("newmtl ")) > -1)
		{
			WavefrontMtl mtl;
			line = line.substr(pos + strlen("newmtl "));
			line.erase(line.find_first_of("#"));
			line.erase(line.find_last_not_of(" \n\r\t")+1);

			mtl.name = line;

			materials.emplace_back(mtl);
		}
		if ((pos = line.find("map_Kd ")) > -1)
		{
			WavefrontMtl &mtl = materials[materials.size() - 1];
			line = line.substr(pos + strlen("map_Kd "));
			line.erase(line.find_first_of("#"));
			line.erase(line.find_last_not_of(" \n\r\t")+1);

			std::wstring wstr(line.begin(),line.end());

			mtl.tex = gpu->CreateGpuTextureFromFile(wstr.c_str());
		}
	}
}
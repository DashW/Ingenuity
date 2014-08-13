#include "stdafx.h"

#include "ObjParser.h"
#include "GeoBuilder.h"
#include "GpuApi.h"
#include <sstream>

namespace Ingenuity {

LocalMesh * ObjParser::CreateMesh(Gpu::Api *gpu)
{
	LocalMesh * mesh = new LocalMesh();
	unsigned vertices = 0;

	if(hasTexCoords)
	{
		VertexBuffer<Vertex_PosNorTex> * v = new VertexBuffer<Vertex_PosNorTex>(refBank.size());

		memcpy(v->GetData(), posNorTexCache.data(), v->GetElementSize() * v->GetLength());

		posNorTexCache.clear();

		mesh->vertexBuffer = v;
	}
	else // ! hasTexCoords
	{
		VertexBuffer<Vertex_PosNor> * v = new VertexBuffer<Vertex_PosNor>(refBank.size());

		memcpy(v->GetData(), posNorCache.data(), v->GetElementSize() * v->GetLength());

		posNorCache.clear();

		mesh->vertexBuffer = v;
	}

	unsigned *k = new unsigned[indexBank.size()];
	memcpy(k, indexBank.data(), indexBank.size() * sizeof(unsigned));

	mesh->indexBuffer = k;
	mesh->numTriangles = indexBank.size() / 3;

	if(!hasNormals && !noNormals)
	{
		GeoBuilder().GenerateNormals(mesh->vertexBuffer, mesh->numTriangles, mesh->indexBuffer);
	}

	// Clear the index and reference banks,
	// ready for the next model
	indexBank.clear();
	refBank.clear();

	return mesh;
}

ObjParser::~ObjParser()
{
	for(unsigned i = 0; i < meshes.size(); ++i)
	{
		delete meshes[i];
	}
}

void ObjParser::ParseMesh(Gpu::Api *gpu, std::string text) // SYNCHRONOUS
{
	std::stringstream stream(text);
	std::string line;

	Reset();

	while(!ready) // Start reading file data
	{
		if(!std::getline(stream, line, '\n')) line = "";
		ParseLine(gpu, line.c_str());
	}
}

void ObjParser::ParseLine(Gpu::Api *gpu, std::string line)
{
	if(line.size() == 0)
	{
		// End of file
		if(refBank.size() > 0)
		{
			meshes.push_back(CreateMesh(gpu));
		}
		ready = true;
		return;
	}

	if(line.find("mtllib ") == 0)
	{
		line = line.substr(strlen("mtllib "));
		line.erase(line.find_last_not_of(" \n\r\t") + 1);

		materialLib = line;
	}
	if(line.find("v ") == 0) // The first character is a v: this line is a vertex
	{
		static float x = 0.0f, y = 0.0f, z = 0.0f;

		if(sscanf_s(line.c_str(), "v %f %f %f", &x, &y, &z) != 3)
		{
			return;
		}

		positionBank.emplace_back(x, y, z);
	}
	if(line.find("vt ") == 0)
	{
		hasTexCoords = true;

		static float u = 0.0f, v = 0.0f;

		sscanf_s(line.c_str(), "vt %f %f", &u, &v);

		v = 1.0f - v;

		texUVBank.emplace_back(u, v);
	}
	if(line.find("vn ") == 0)
	{
		hasNormals = true;

		static float x = 0.0f, y = 0.0f, z = 0.0f;

		sscanf_s(line.c_str(), "vn %f %f %f", &x, &y, &z);

		normalBank.emplace_back(x, y, z);
	}
	if(line.find("usemtl ") == 0)
	{
		if(refBank.size() > 0) meshes.push_back(CreateMesh(gpu));

		line = line.substr(strlen("usemtl "));
		line.erase(line.find_last_not_of(" \n\r\t") + 1);

		materials.emplace_back(line);
	}
	if(line.find("f ") == 0)
	{
		static int
			a = 0, ta = 0, na = 0,
			b = 0, tb = 0, nb = 0,
			c = 0, tc = 0, nc = 0,
			d = 0, td = 0, nd = 0;

		if(hasTexCoords)
		{
			if(hasNormals)
			{
				if(sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &a, &ta, &na, &b, &tb, &nb, &c, &tc, &nc) != 9)
				{
					// LOG: Faces are incorrectly formatted
					return;
				}
			}
			else
			{
				int numVerts = sscanf_s(line.c_str(), "f %d/%d %d/%d %d/%d %d/%d", &a, &ta, &b, &tb, &c, &tc, &d, &td);
				if(numVerts == 8) // QUAD
				{
					AddRef(c, tc, 1);
					AddRef(d, td, 1);
					AddRef(a, ta, 1);
				}
				else if(numVerts != 6)
				{
					// LOG: Faces are incorrectly formatted
					return;
				}
			}
		}
		else // ! hasTexCoords
		{
			if(sscanf_s(line.c_str(), "f %d//%d %d//%d %d//%d", &a, &na, &b, &nb, &c, &nc) != 6)
			{
				// LOG: Faces are incorrectly formatted
				return;
			}
		}

		AddRef(a, ta, na);
		AddRef(b, tb, nb);
		AddRef(c, tc, nc);
	}
}

void ObjParser::AddRef(unsigned pos, unsigned tex, unsigned nor)
{
	pos -= (pos > 0 ? 1 : 0);
	tex -= (tex > 0 ? 1 : 0);
	nor -= (nor > 0 ? 1 : 0);

	if(consolidate)
	{
		unsigned long priorIndex = (pos * (normalBank.size() + 1) * (texUVBank.size() + 1))
			+ (tex * (normalBank.size() + 1)) + nor;

		std::hash_map<unsigned long, unsigned int>::iterator prior = refMap.find(priorIndex);

		if(prior != refMap.end())
		{
			indexBank.push_back(prior->second);
		}
		else
		{
			refBank.emplace_back(pos, tex, nor);
			indexBank.push_back(refBank.size() - 1);
			refMap[priorIndex] = refBank.size() - 1;

			if(hasTexCoords)
			{
				if(hasNormals)
				{
					posNorTexCache.emplace_back(positionBank[pos], normalBank[nor], texUVBank[tex]);
				}
				else
				{
					posNorTexCache.emplace_back(positionBank[pos], glm::vec3(0.0f, 1.0f, 0.0f), texUVBank[tex]);
				}
			}
			else // ! hasTexCoords
			{
				posNorCache.emplace_back(positionBank[pos], normalBank[nor]);
			}
		}
	}
	else
	{
		refBank.emplace_back(pos, tex, nor);
		indexBank.push_back(refBank.size() - 1);

		if(hasTexCoords)
		{
			if(hasNormals)
			{
				posNorTexCache.emplace_back(positionBank[pos], normalBank[nor], texUVBank[tex]);
			}
			else
			{
				posNorTexCache.emplace_back(positionBank[pos], glm::vec3(0.0f, 1.0f, 0.0f), texUVBank[tex]);
			}
		}
		else // ! hasTexCoords
		{
			posNorCache.emplace_back(positionBank[pos], normalBank[nor]);
		}
	}
}

void ObjParser::Reset()
{
	hasTexCoords = false;
	hasNormals = false;
	ready = false;
}

} // namespace Ingenuity

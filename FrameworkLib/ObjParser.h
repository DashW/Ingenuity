#pragma once

#include "MtlParser.h"
#include "GpuVertices.h"
#include <string>
#include <vector>
#include <hash_map>
#include "../Third Party/glm-0.9.4.1/glm/ext.hpp"

namespace Ingenuity {

struct LocalMesh;

namespace Gpu {
class Api;
}

class ObjParser
{
protected:
	//struct Vector3 {
	//	float x, y, z;
	//	Vector3(float x, float y, float z) 
	//		: x(x), y(y), z(z) {}
	//};

	//struct Vector2 {
	//	float u, v;
	//	Vector2(float u, float v)
	//		: u(u), v(v) {}
	//};

	struct VertexRef {
		int vertex, texture, normal;
		VertexRef(int v, int t, int n)
			: vertex(v), texture(t), normal(n) {}
	};

	bool ready;
	bool noNormals;
	bool consolidate;

	bool hasTexCoords;
	bool hasNormals;

	std::vector<glm::vec3> positionBank;
	std::vector<glm::vec3> normalBank;
	std::vector<glm::vec2> texUVBank;
	std::vector<VertexRef> refBank;
	std::vector<unsigned> indexBank;
	std::hash_map<unsigned long, unsigned int> refMap;

	std::vector<Vertex_PosNor> posNorCache;
	std::vector<Vertex_PosNorTex> posNorTexCache;

	std::string materialLib;
	std::vector<std::string> materials;
	std::vector<LocalMesh*> meshes;

	LocalMesh * CreateMesh(Gpu::Api * gpu);

	void AddRef(unsigned pos, unsigned tex, unsigned nor);

public:
	ObjParser(bool noNormals = false, bool consolidate = false) :
		ready(false),
		noNormals(noNormals),
		consolidate(consolidate),
		hasTexCoords(false),
		hasNormals(false) {}
	virtual ~ObjParser();

	virtual void ParseMesh(Gpu::Api *gpu, std::string text);
	virtual void ParseLine(Gpu::Api *gpu, std::string line);

	virtual void Reset();

	virtual bool hasMaterialLib() { return materialLib.size() != 0; }

	virtual const char* GetMaterialLib() { return materialLib.c_str(); }

	virtual unsigned int GetNumMeshes() { return meshes.size(); }
	virtual const char * GetMaterial(int index) { return materials[index].c_str(); }
	virtual LocalMesh * GetMesh(int index) { return meshes[index]; }

	virtual bool IsReady() { return ready; }
	virtual void SetNoNormals(bool noNormals) { this->noNormals = noNormals; }
	virtual void SetConsolidate(bool consolidate) { this->consolidate = consolidate; }
};

} // namespace Ingenuity

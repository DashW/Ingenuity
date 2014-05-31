#pragma once

#include "GpuApi.h"
#include "../Third Party/glm-0.9.4.1/glm/glm.hpp"

namespace Ingenuity {

class IsoSurface
{
public:
	struct Metaball
	{
		glm::vec3 position;
		float squaredRadius;

		Metaball() : squaredRadius(1.0f) {}
	};

private:
	VertexBuffer<Vertex_PosNor> cubeVertexBuffer;
	VertexBuffer<Vertex_PosNor> * outVertexBuffer;
	float * fieldValues;
	float threshold;

	struct Cube
	{
		unsigned vbIndices[8];
	};

	unsigned numCubes;
	Cube * cubes;

	std::vector<Metaball> metaballs;

	Gpu::Mesh * dynamicMesh;

public:
	IsoSurface(unsigned gridSize, Gpu::Api * gpu);
	~IsoSurface();

	void SetThreshold(float threshold) { this->threshold = threshold; }

	void AddMetaball(glm::vec3 position = glm::vec3(), float sqrRadius = 1.0f)
	{
		metaballs.emplace_back();
		metaballs.back().position = position;
		metaballs.back().squaredRadius = sqrRadius;
	}
	unsigned GetNumMetaballs() { return metaballs.size(); }
	Metaball * GetMetaball(unsigned index) {
		return (index < metaballs.size() ? &metaballs[index] : 0);
	}
	void ClearMetaballs() { metaballs.clear(); }

	void UpdateObjects();
	void UpdateMesh(Gpu::Api * gpu);
	Gpu::Mesh * GetMesh() const { return this->dynamicMesh; }

private:
	static const int verticesAtEndsOfEdges[24];
	static const int edgeTable[256];
	static const int triTable[256][16];
};

} // namespace Ingenuity

#pragma once

#include "GpuApi.h"
#include "GeoStructs.h"
#include <debugapi.h>
#include <vector>
#include <functional>
#include <map>

namespace Ingenuity {

struct LocalMesh
{
	IVertexBuffer * vertexBuffer;
	unsigned * indexBuffer;
	unsigned numTriangles;
	Gpu::Mesh * boundDynamicMesh;

	LocalMesh() : vertexBuffer(0), indexBuffer(0), numTriangles(0), boundDynamicMesh(0) {}
	LocalMesh(IVertexBuffer * vertices, unsigned * indices, unsigned numIndices) :
		vertexBuffer(vertices),
		indexBuffer(indices),
		numTriangles(numIndices / 3), // FIXME - SHOULD USE NUMTRIANGLES TO AVOID UNSAFE DIVISIONS!!!
		boundDynamicMesh(0) {}

	virtual ~LocalMesh() {
		if(vertexBuffer) delete vertexBuffer; vertexBuffer = 0;
		if(indexBuffer) delete[] indexBuffer; indexBuffer = 0;

		if(boundDynamicMesh) delete boundDynamicMesh; boundDynamicMesh = 0;
	}

	unsigned GetNumIndices() { return numTriangles * 3; }

	Gpu::Mesh * ToGpuMesh(Gpu::Api * gpu, bool dynamic = false, bool bind = false)
	{
		if(!(gpu && vertexBuffer && indexBuffer)) return 0;

		Gpu::Mesh * createdMesh = gpu->CreateGpuMesh(vertexBuffer, numTriangles, indexBuffer, dynamic);
		if(bind) boundDynamicMesh = createdMesh;
		return createdMesh;
	}

	Gpu::Mesh * GpuOnly(Gpu::Api * gpu, bool dynamic = false)
	{
		Gpu::Mesh * result = ToGpuMesh(gpu, dynamic, false);
		delete this;
		return result;
	}

	void UpdateBoundMesh(Gpu::Api * gpu)
	{
		if(boundDynamicMesh)
		{
			gpu->UpdateDynamicMesh(boundDynamicMesh, vertexBuffer);
			gpu->UpdateDynamicMesh(boundDynamicMesh, numTriangles, indexBuffer);
		}
	}
};

class GeoBuilder
{
	struct Triangle
	{
		unsigned a, b, c;
	};

	struct Diagonal
	{
		unsigned a, b;
		Diagonal(unsigned a, unsigned b) : a(a), b(b) {}
		bool operator < (Diagonal & other)
		{
			return b - a < other.b - other.a;
		}
	};

	IVertexBuffer* BuildVertexBufferCylinder(
		const float height, const float radius, const unsigned sectors,
		const unsigned stacks, bool texCoords = false, const bool sphere = false);
	unsigned* BuildIndexBufferCylinder(
		const unsigned sectors, const unsigned stacks, unsigned& length);

	void GenerateSortedIndices(Path & path, std::vector<unsigned> & sortedIndices);
	void FixPathCrossover(double crossX, double crossY, unsigned edge1, unsigned edge2, Path & path);
	void FixPathCrossovers(Path & original);
	LocalMesh * CombineLocalMeshes(LocalMesh ** meshes, unsigned numMeshes);
	LocalMesh * BuildDiagonalDebugMesh(Path & path, std::vector<Diagonal> & diagonals);
	LocalMesh * BuildMonotoneDebugMesh(Path & path, std::vector<unsigned> * monotones, unsigned numMonotones);
	void SweepDiagonals(Path & path, std::vector<Diagonal> & diagonals);
	unsigned BuildMonotones(Path & path, std::vector<Diagonal> & diagonals, std::vector<unsigned> * monotones);
	void TriangulateMonotone(Path & path, std::vector<unsigned> & monotone, std::vector<Triangle> & triangles);

	static Path * staticPath;
	static bool ComparePathPoints(const unsigned first, const unsigned second)
	{
		double difference = staticPath->points[first].y - staticPath->points[second].y;
		if(difference == 0.0f)
		{
			//if(IsDebuggerPresent()) DebugBreak();
			staticPath->points[first].y += FLT_EPSILON * 1000.0f;
			difference = staticPath->points[first].y - staticPath->points[second].y;
		}
		return difference < 0.0f;
	}
	static std::vector<unsigned> * staticMonotone;
	static bool CompareMonotonePathPoints(unsigned first, unsigned second)
	{
		double difference = staticPath->points[staticMonotone->at(first)].y - staticPath->points[staticMonotone->at(second)].y;
		if(difference == 0.0f)
		{
			//if(IsDebuggerPresent()) DebugBreak();
			staticPath->points[staticMonotone->at(first)].y += FLT_EPSILON * 1000.0f;
			difference = staticPath->points[staticMonotone->at(first)].y - staticPath->points[staticMonotone->at(second)].y;
		}
		return difference < 0.0f;
	}

	struct ComparePathEdges : std::binary_function < Path::Edge, Path::Edge, bool > {
		bool operator() (const Path::Edge & first, const Path::Edge & second) const
		{
			return first.xPos < second.xPos;
		}
	};

	VertApi vtx;
	typedef std::multimap<double, Path::Edge*, std::greater<double>> EdgeTree;
	static const double crossoverFixingOffset;
	unsigned lastDetectedCrossovers;

public:
	virtual ~GeoBuilder() {}

	LocalMesh * BuildCube();
	LocalMesh * BuildSkyCube();
	LocalMesh * BuildCylinder(float radius, float length, unsigned sectors, unsigned stacks, bool texCoords = false);
	LocalMesh * BuildSphere(float radius, unsigned sectors, unsigned stacks, bool texCoords = false);
	LocalMesh * BuildGrid(float width, float depth, unsigned columns, unsigned rows, Gpu::Rect* textureRect = 0, bool tangents = false);

	void GenerateNormals(IVertexBuffer * buffer, unsigned numTriangles, unsigned * indexData);
	void GenerateTangents(IVertexBuffer *& buffer, unsigned numTriangles, unsigned * indexData);
	void GenerateRotationalTangents(IVertexBuffer *& buffer, unsigned numTriangles, unsigned * indexData);

	Gpu::BoundingBox GenerateBoundingBox(IVertexBuffer * buffer);
	Gpu::BoundingSphere GenerateBoundingSphere(IVertexBuffer * buffer);

	// 2D Models
	LocalMesh * BuildRect(float x, float y, float width, float height, bool texCoords = false);
	LocalMesh * BuildRectStroke(float x, float y, float width, float height, float strokeWidth);
	LocalMesh * BuildEllipse(float cx, float cy, float rx, float ry);
	LocalMesh * BuildEllipseStroke(float cx, float cy, float rx, float ry, float strokeWidth);

	enum PathDebugFlag
	{
		DebugMonotones,
		DebugDiagonals,
		DebugCollisions,
		None
	};

	enum StrokeCornerType
	{
		CornerMiter,
		CornerRound,
		CornerBevel
	};
	enum StrokeCapType
	{
		CapButt,
		CapRound,
		CapSquare
	};

	LocalMesh * BuildPath(Path::Point * points, unsigned numPoints, PathDebugFlag debugFlag = None);
	LocalMesh * BuildStroke(Path::Point * points, unsigned numPoints, float strokeWidth,
		StrokeCornerType cornerType = CornerMiter, StrokeCapType capType = CapButt, float miterLimit = 4.0f);

	LocalMesh * BuildPathPoly2Tri(Path::Point * points, unsigned numPoints);
};

struct TangentGenerator : public Steppable
{
	LocalMesh * mesh;
	unsigned vbIndex;
	unsigned triIndex;

	struct VertexNode
	{
		glm::vec3 tangent;
		glm::vec3 bitangent;
		VertexNode() : tangent(0.0f, 0.0f, 0.0f), bitangent(0.0f, 0.0f, 0.0f) {}
	};

	VertexNode *nodes;
	VertexBuffer<Vertex_PosNorTanTex> * newBuffer;

	TangentGenerator(LocalMesh * mesh) : mesh(mesh), vbIndex(0), triIndex(0), nodes(0) {}

	virtual void Step() override;
	virtual float GetProgress() { return float(triIndex + vbIndex) / float(mesh->numTriangles + mesh->vertexBuffer->GetLength()); }
	virtual bool IsFinished() { return vbIndex >= mesh->vertexBuffer->GetLength(); }

};

} // namespace Ingenuity

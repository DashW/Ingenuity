#pragma once

#include "GpuStructs.h"

namespace Ingenuity
{

enum VertexType
{
	VertexType_Pos = 0,
	VertexType_PosCol,
	VertexType_PosNor,
	VertexType_PosTex,
	VertexType_PosNorTex,
	VertexType_PosNorTanTex,

	VertexType_Count
};

struct Vertex_Pos
{
	glm::vec3 position;

	static VertexType GetType() {
		return VertexType_Pos;
	}

	Vertex_Pos() {}
	Vertex_Pos(float x, float y, float z) :
		position(x, y, z) {}
};

struct Vertex_PosCol
{
	glm::vec3 position;
	glm::vec3 color;

	static VertexType GetType() {
		return VertexType_PosCol;
	}

	Vertex_PosCol() {}
	Vertex_PosCol(float x, float y, float z,
		float r, float g, float b) :
		position(x, y, z), color(r, g, b) {}
};

struct Vertex_PosNor
{
	glm::vec3 position;
	glm::vec3 normal;

	static VertexType GetType() {
		return VertexType_PosNor;
	}

	Vertex_PosNor() {}
	Vertex_PosNor(glm::vec3 pos, glm::vec3 nor) :
		position(pos), normal(nor) {}
	Vertex_PosNor(float x, float y, float z,
		float u, float v, float w) :
		position(x, y, z), normal(u, v, w) {}
};

struct Vertex_PosTex
{
	glm::vec3 position;
	glm::vec2 texCoord;

	static VertexType GetType() {
		return VertexType_PosTex;
	}

	Vertex_PosTex() {}
	Vertex_PosTex(float x, float y, float z,
		float tx, float ty) :
		position(x, y, z), texCoord(tx, ty) {}
};

struct Vertex_PosNorTex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;

	static VertexType GetType() {
		return VertexType_PosNorTex;
	}

	Vertex_PosNorTex() {}
	Vertex_PosNorTex(glm::vec3 pos, glm::vec3 nor, glm::vec2 tex) :
		position(pos), normal(nor), texCoord(tex) {}
	Vertex_PosNorTex(float x, float y, float z,
		float u, float v, float w,
		float tx, float ty) :
		position(x, y, z), normal(u, v, w), texCoord(tx, ty) {}
};

struct Vertex_PosNorTanTex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	float tanChirality;
	glm::vec2 texCoord;

	static VertexType GetType() {
		return VertexType_PosNorTanTex;
	}

	Vertex_PosNorTanTex() {}
	Vertex_PosNorTanTex(float x, float y, float z,
		float norx, float nory, float norz,
		float tanx, float tany, float tanz, float tanc,
		float tx, float ty) :
		position(x, y, z), normal(norx, nory, norz),
		tangent(tanx, tany, tanz), tanChirality(tanc),
		texCoord(tx, ty) {}
};

class IVertexBuffer
{
protected:
	const unsigned length;
public:
	IVertexBuffer(unsigned length) : length(length) {}
	virtual ~IVertexBuffer() {}
	virtual void* GetData() = 0;
	virtual unsigned GetElementSize() = 0;
	virtual unsigned GetLength() { return length; };
	virtual VertexType GetVertexType() = 0;
};

template <class VERTEX>
class VertexBuffer : public IVertexBuffer
{
	VertexType type;

	VERTEX * buffer;
	//const unsigned numVertices;

public:
	VertexBuffer(unsigned length) :
		IVertexBuffer(length), type(VERTEX::GetType())
	{
		buffer = new VERTEX[length];
	}
	virtual ~VertexBuffer()
	{
		delete[length] buffer;
	}
	virtual void * GetData() override { return buffer; }
	virtual unsigned GetElementSize() override { return sizeof(VERTEX); }
	virtual VertexType GetVertexType() override { return type; }

	void Set(unsigned index, VERTEX & vertex) { buffer[index] = vertex; }
	VERTEX & Get(unsigned index) { return buffer[index]; }
};

enum InstanceType
{
	InstanceType_None,
	InstanceType_Pos,
	InstanceType_PosCol,
	InstanceType_PosSca,

	InstanceType_Count
};

struct Instance_Pos
{
	glm::vec3 position;
	static const InstanceType type = InstanceType_Pos;

	void UpdateFromModel(Gpu::Model * model)
	{
		position = model->position;
	}
};

struct Instance_PosCol
{
	glm::vec3 position;
	glm::vec4 color;
	static const InstanceType type = InstanceType_PosCol;

	void UpdateFromModel(Gpu::Model * model)
	{
		position = model->position;
		color = model->color;
	}
};

struct Instance_PosSca
{
	glm::vec3 position;
	glm::vec3 scale;
	static const InstanceType type = InstanceType_PosSca;

	void UpdateFromModel(Gpu::Model * model)
	{
		position = model->position;
		scale = model->scale;
	}

	//Instance_PosSca() : scale(1.0f) {}
};

class VertApi {
public:

	static unsigned GetVertexSize(VertexType type)
	{
		switch(type)
		{
		case VertexType_Pos:
			return sizeof(Vertex_Pos);
		case VertexType_PosCol:
			return sizeof(Vertex_PosCol);
		case VertexType_PosNor:
			return sizeof(Vertex_PosNor);
		case VertexType_PosTex:
			return sizeof(Vertex_PosTex);
		case VertexType_PosNorTex:
			return sizeof(Vertex_PosNorTex);
		case VertexType_PosNorTanTex:
			return sizeof(Vertex_PosNorTanTex);
		};
		return 0;
	}

	static const char * GetVertexName(VertexType type)
	{
		switch(type)
		{
		case VertexType_Pos:          return "Pos";
		case VertexType_PosCol:       return "PosCol";
		case VertexType_PosNor:       return "PosNor";
		case VertexType_PosTex:       return "PosTex";
		case VertexType_PosNorTex:    return "PosNorTex";
		case VertexType_PosNorTanTex: return "PosNorTanTex";
		default:                      return 0;
		}
	}

	static unsigned GetInstanceSize(InstanceType type)
	{
		switch(type)
		{
		case InstanceType_Pos:
			return sizeof(Instance_Pos);
		case InstanceType_PosCol:
			return sizeof(Instance_PosCol);
		case InstanceType_PosSca:
			return sizeof(Instance_PosSca);
		}
		return 0;
	}

	static unsigned GetTechniqueKey(VertexType vType, InstanceType iType) { return (iType * VertexType_Count) + vType; }
};

} // namespace Ingenuity

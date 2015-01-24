#pragma once

#include "GpuStructs.h"

namespace Ingenuity
{

struct Vertex_Pos;
struct Vertex_PosCol;
struct Vertex_PosNor;
struct Vertex_PosTex;
struct Vertex_PosNorTex;
struct Vertex_PosNorTanTex;

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
static const char * VertexType_Names[VertexType_Count] =
{
	"Pos",
	"PosCol",
	"PosNor",
	"PosTex",
	"PosNorTex",
	"PosNorTanTex"
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

	void Transform(const glm::mat4 & matrix, const glm::mat4 & invTraMatrix)
	{
		position = glm::vec3(matrix * glm::vec4(position, 1.0f));
	}
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

	void Transform(const glm::mat4 & matrix, const glm::mat4 & invTraMatrix)
	{
		position = glm::vec3(matrix * glm::vec4(position, 1.0f));
	}
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

	void Transform(const glm::mat4 & matrix, const glm::mat4 & invTraMatrix)
	{
		position = glm::vec3(matrix * glm::vec4(position, 1.0f));
		normal = glm::normalize(glm::vec3(invTraMatrix * glm::vec4(normal, 1.0f)));
	}
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

	void Transform(const glm::mat4 & matrix, const glm::mat4 & invTraMatrix)
	{
		position = glm::vec3(matrix * glm::vec4(position, 1.0f));
	}
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

	void Transform(const glm::mat4 & matrix, const glm::mat4 & invTraMatrix)
	{
		position = glm::vec3(matrix * glm::vec4(position, 1.0f));
		normal = glm::normalize(glm::vec3(invTraMatrix * glm::vec4(normal, 1.0f)));
	}
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

	void Transform(const glm::mat4 & matrix, const glm::mat4 & invTraMatrix)
	{
		position = glm::vec3(matrix * glm::vec4(position, 1.0f));
		normal = glm::normalize(glm::vec3(invTraMatrix * glm::vec4(normal, 1.0f)));
		// TODO: NEED TO ADD TRANSFORMATION FOR THE TANGENT!!
	}
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
	virtual void Transform(const glm::mat4 & matrix) = 0;
};

template <class VERTEX>
class VertexBuffer : public IVertexBuffer
{
	VertexType type;

	VERTEX * buffer;

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
	virtual void Transform(const glm::mat4 & matrix) override
	{
		const glm::mat4 invTraMatrix = glm::inverse(glm::transpose(matrix));
		for(unsigned i = 0; i < length; i++)
			buffer[i].Transform(matrix, invTraMatrix);
	}

	void Set(unsigned index, VERTEX & vertex) { buffer[index] = vertex; }
	VERTEX & Get(unsigned index) { return buffer[index]; }
};

struct Instance_Pos;
struct Instance_PosCol;
struct Instance_PosSca;
struct Instance_PosRotSca;

enum InstanceType
{
	InstanceType_None,
	InstanceType_Pos,
	InstanceType_PosCol,
	InstanceType_PosSca,
	InstanceType_PosRotSca,

	InstanceType_Count
};
static const char * InstanceType_Names[InstanceType_Count] = 
{
	"",
	"Pos",
	"PosCol",
	"PosSca",
	"PosRotSca"
};

struct Instance_Pos
{
	glm::vec3 position;
	static const InstanceType type = InstanceType_Pos;

	void UpdateFromModel(Gpu::Model * model)
	{
		if(model->useMatrix) return;
		position = glm::vec3(model->position);
	}
};

struct Instance_PosCol
{
	glm::vec3 position;
	glm::vec4 color;
	static const InstanceType type = InstanceType_PosCol;

	void UpdateFromModel(Gpu::Model * model)
	{
		if(model->useMatrix) return;
		position = glm::vec3(model->position);
		color = model->color;
	}

	Instance_PosCol() : color(1.0f) {}
};

struct Instance_PosSca
{
	glm::vec3 position;
	glm::vec3 scale;
	static const InstanceType type = InstanceType_PosSca;

	void UpdateFromModel(Gpu::Model * model)
	{
		if(model->useMatrix) return;
		position = glm::vec3(model->position);
		scale = glm::vec3(model->scale);
	}

	Instance_PosSca() : scale(1.0f) {}
};

struct Instance_PosRotSca
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	static const InstanceType type = InstanceType_PosRotSca;

	void UpdateFromModel(Gpu::Model * model)
	{
		if(model->useMatrix) return;
		position = glm::vec3(model->position);
		rotation = glm::vec3(model->rotation);
		scale = glm::vec3(model->scale);
	}

	Instance_PosRotSca() : scale(1.0f) {}
};

static const unsigned VertexType_Sizes[VertexType_Count] =
{
	sizeof(Vertex_Pos),
	sizeof(Vertex_PosCol),
	sizeof(Vertex_PosNor),
	sizeof(Vertex_PosTex),
	sizeof(Vertex_PosNorTex),
	sizeof(Vertex_PosNorTanTex)
};

static const unsigned InstanceType_Sizes[InstanceType_Count] =
{
	0,
	sizeof(Instance_Pos),
	sizeof(Instance_PosCol),
	sizeof(Instance_PosSca),
	sizeof(Instance_PosRotSca)
};

class VertApi {

public:

	inline static unsigned GetVertexSize(VertexType type)
	{
		return VertexType_Sizes[type];
	}

	inline static const char * GetVertexName(VertexType type)
	{
		return VertexType_Names[type];
	}

	inline static unsigned GetInstanceSize(InstanceType type)
	{
		return InstanceType_Sizes[type];
	}

	inline static const char * GetInstanceName(InstanceType type)
	{
		return InstanceType_Names[type];
	}

	static unsigned GetTechniqueKey(VertexType vType, InstanceType iType) { return (iType * VertexType_Count) + vType; }
};

} // namespace Ingenuity

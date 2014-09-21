#pragma once

#include <string>
#include <map>

namespace Ingenuity {

struct ScriptPtrType
{
	enum Value
	{
		Unknown,

		GpuComplexModel,
		GpuEffect,
		GpuTexture,
		GpuCubeMap,
		GpuVolumeTexture,
		GpuCamera,
		GpuFont,
		GpuLight,
		GpuDrawSurface,
		GpuScene,
		GpuShader,
		GpuInstanceBuffer,

		FloatArray,
		HeightParser,
		ImageBuffer,
		AudioItem,
		IsoSurface,
		SVGParser,
		PhysicsWorld,
		PhysicsObject,
		PhysicsMaterial,
		PhysicsRagdoll,
		LeapHelper
	};
};

struct IDeletingPtr
{
	void * ptr;
	unsigned refs;
	ScriptPtrType::Value type;

	IDeletingPtr(void * ptr) : ptr(ptr), refs(0), type(ScriptPtrType::Unknown) {}
	virtual ~IDeletingPtr() {}
	void IncRef() { refs++; };
	void DecRef() { if(--refs < 1){ Delete(); delete this; } }
	virtual void Delete() = 0;
};
template <class CLASS>
struct DeletingPtr : public IDeletingPtr
{
	DeletingPtr(CLASS * ptr) : IDeletingPtr(ptr) {}
	virtual ~DeletingPtr() {}
	virtual void Delete() override { delete static_cast<CLASS*>(ptr); }
};
struct NonDeletingPtr : public IDeletingPtr
{
	NonDeletingPtr(void * ptr) : IDeletingPtr(ptr) {}
	NonDeletingPtr(void * ptr, ScriptPtrType::Value type) : IDeletingPtr(ptr) { this->type = type; }
	virtual ~NonDeletingPtr() {}
	virtual void Delete() override {}
};

struct ScriptParam
{
	enum Type
	{
		NONE = 0,
		BOOL,
		INT,
		FLOAT,
		DOUBLE,
		STRING,
		POINTER,
		FUNCTION,
		MAPREF
	};

	Type type;

	double nvalue;
	const char* svalue;
	IDeletingPtr * pvalue;

	ScriptParam(Type t, double n)
		: type(t), nvalue(n)
	{}
	ScriptParam(Type t, const char * s)
		: type(t), svalue(s)
	{}
	template<class CLASS>
	ScriptParam(CLASS * p, ScriptPtrType::Value ptrType)
		: type(p != 0 ? POINTER : NONE)
	{
		if(p)
		{
			pvalue = new DeletingPtr<CLASS>(p);
			pvalue->type = ptrType;
			pvalue->IncRef();
		}
	}
	ScriptParam(IDeletingPtr * p)
		: type(p != 0 ? POINTER : NONE), pvalue(p)
	{
		if(p)
		{
			pvalue->IncRef();
		}
	}
	ScriptParam()
		: type(NONE)
	{
		nvalue = 0.0;
	}
	ScriptParam(const ScriptParam & other) :
		type(other.type), nvalue(other.nvalue), svalue(other.svalue),
		pvalue(other.pvalue)
	{
		if(type == POINTER)
		{
			pvalue->IncRef();
		}
	}
	ScriptParam& operator= (const ScriptParam & other)
	{
		if(this == &other) return *this; // dangerous?

		if(other.type == POINTER)
		{
			other.pvalue->IncRef();
		}
		if(this->type == POINTER)
		{
			pvalue->DecRef();
		}

		type = other.type;
		nvalue = other.nvalue;
		svalue = other.svalue;
		pvalue = other.pvalue;

		return *this;
	}
	//const bool operator< (const ScriptParam & other) const
	//{
	//	if(type < other.type)
	//	{
	//		return true;
	//	}
	//	if(type > other.type)
	//	{
	//		return false;
	//	}
	//	switch(type)
	//	{
	//	case NONE:
	//		return false;
	//	case BOOL:
	//	case INT:
	//	case FLOAT:
	//	case DOUBLE:
	//		return nvalue < other.nvalue;
	//	case STRING:
	//		return bool(-strcmp(svalue, other.svalue));
	//	case POINTER:
	//		return pvalue < other.pvalue;
	//	default:
	//		return true;
	//	}
	//}
	~ScriptParam()
	{
		if(type == POINTER)
		{
			pvalue->DecRef();
		}
	}

	bool CheckPointer(ScriptPtrType::Value ptrType) const
	{
		return type == POINTER && pvalue->type == ptrType;
	}
	bool IsNumber() const
	{
		return type == INT || type == FLOAT || type == DOUBLE;
	}
};

} // namespace Ingenuity

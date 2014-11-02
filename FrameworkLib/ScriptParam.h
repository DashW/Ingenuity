#pragma once

#include <string>
#include <map>

namespace Ingenuity {

struct IDeletingPtr
{
	void * ptr;
	unsigned refs;
	unsigned type;

	IDeletingPtr(void * ptr) : ptr(ptr), refs(0), type(0) {}
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
	NonDeletingPtr(void * ptr, unsigned type) : IDeletingPtr(ptr) { this->type = type; }
	virtual ~NonDeletingPtr() {}
	virtual void Delete() override {}
};
struct BufferCopyPtr : public IDeletingPtr
{
	unsigned size;
	BufferCopyPtr(void * ptr, unsigned size) : IDeletingPtr(new char[size]), size(size) { memcpy(this->ptr, ptr, size); }
	BufferCopyPtr(void * ptr, unsigned size, unsigned type) : BufferCopyPtr(ptr, size) { this->type = type; }
	virtual ~BufferCopyPtr() {}
	virtual void Delete() override { delete[] static_cast<char*>(ptr); }
};

struct ScriptParam
{
	enum Type
	{
		NONE = 0,
		BOOL,
		INT,
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
	ScriptParam(CLASS * p, unsigned ptrType)
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
	~ScriptParam()
	{
		if(type == POINTER)
		{
			pvalue->DecRef();
		}
	}

	bool CheckPointer(unsigned ptrType) const
	{
		return type == POINTER && pvalue->type == ptrType;
	}
	bool IsNumber() const
	{
		return type == INT || type == DOUBLE;
	}
	template <class CLASS> CLASS * GetPointer()
	{
		return static_cast<CLASS*>(pvalue->ptr);
	}
};

} // namespace Ingenuity

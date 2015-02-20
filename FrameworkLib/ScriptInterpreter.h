#pragma once

#include <stdarg.h>
#include "ScriptLogger.h"
#include "ScriptParam.h"
#include <vector>

namespace Ingenuity {

class ScriptInterpreter;

struct ScriptCallbackMeta
{
	const char * name;
	//void(*call)(ScriptInterpreter*);
	const char * help;

	ScriptCallbackMeta(const char * name, const char * help = "")
		: name(name), help(help) {}
};

//struct ScriptModule
//{
//	const char * name;
//	const char * help;
//
//	ScriptModule(const char * name, const char * help)
//		: name(name), help(help) {}
//};

class RealtimeApp;

class ScriptInterpreter
{
	static const int MAX_PARAMS = 20;
	int dependencies;
	int numParams;
	ScriptParam params[MAX_PARAMS];
	ScriptLogger logger;

	bool inError;
	bool initialised;

protected:
	RealtimeApp * app;

	ScriptInterpreter(RealtimeApp * app) :
		dependencies(0),
		numParams(0),
		inError(false),
		initialised(false),
		app(app)
	{}

	virtual void FunctionCalled(const char * functionName) = 0;

	inline void SetParam(int index, ScriptParam param)
	{
		if(index > -1 && index < numParams)
		{
			params[index] = param;
		}
	}

	inline ScriptParam GetParam(int index)
	{
		if(index > -1 && index < numParams)
			return params[index];
		return ScriptParam();
	}

	inline void SetNumParams(int numParams)
	{
		if(numParams > MAX_PARAMS)
		{
			ThrowError("Function cannot accept more than 20 parameters!");
		}
		else if(numParams > -1)
		{
			this->numParams = numParams;
		}
	}

	void SetError(bool inError) { this->inError = inError; }
	void SetInitialised() { initialised = true; }

public:
	typedef void(*ScriptCallback)(ScriptInterpreter*);

	std::vector<ScriptCallbackMeta> callbackMeta;
	//std::vector<ScriptModule> modules; // I'm not quite sure where I'm going with this...

	virtual ~ScriptInterpreter() {}

	enum Operator
	{
		Call,
		Add,
		Sub,
		Mul,
		Div,
		Pow,
		Neg,
		Equals,
		ToString,
		IndexGet,
		IndexSet,
		Length
	};

	enum SpecialPtrType
	{
		TypeVector4,
		TypeMatrix4,
		TypeFloatArray
	};

	virtual bool LoadScript(const char * data, unsigned dataSize, const char * filename, const char * moduleName = 0) = 0;
	virtual void ThrowError(const char * error) = 0;
	virtual void RunCommand(const char * command) = 0; // This and LoadScript now do the same thing, should be merged?
	//virtual void ReloadScript(const wchar_t * filename) = 0;

	bool IsInError() { return inError; }
	bool IsInitialised() { return initialised && dependencies == 0; }

	inline void PushParam(ScriptParam param)
	{
		if(numParams < MAX_PARAMS)
		{
			params[numParams] = param;
			++numParams;
		}
	}

	inline ScriptParam PopParam()
	{
		if(numParams > 0)
		{
			--numParams;
			ScriptParam result = params[numParams];
			params[numParams] = ScriptParam();
			return result;
		}
		return ScriptParam();
	}

	inline int NumParams() { return numParams; }

	inline void ClearParams() { while(numParams > 0) params[--numParams] = ScriptParam(); }

	virtual bool HasFunction(const char * functionName) = 0;

	//virtual ScriptParam GetGlobal(const char * globalName) = 0;

	void CallFunction(const char* functionName /*, ...*/)
	{
		if(!inError)
		{
			FunctionCalled(functionName);
		}
	}

	virtual void CallFunction(ScriptParam function) = 0;

	inline ScriptLogger & GetLogger() { return logger; }
	inline RealtimeApp * GetApp() { return app; }

	virtual unsigned RegisterPointerType(unsigned structSize = 0) = 0;
	virtual void RegisterCallback(const char * name, ScriptCallback callback) = 0;
	virtual void RegisterCallback(const char * name, ScriptCallback callback, const char * help)
	{
		for(unsigned i = 0; i < callbackMeta.size(); ++i)
		{
			if(strcmp(name, callbackMeta[i].name) == 0)
			{
				std::string error = "Module function registered twice: ";
				error += name;
				ThrowError(error.c_str());
				break;
			}
		}
		callbackMeta.push_back(ScriptCallbackMeta(name, help));
		RegisterCallback(name, callback);
	}
	//virtual void RegisterModule(ScriptModule & module) = 0;
	virtual void RegisterMathObjects() = 0;

	virtual void RegisterMethod(unsigned ptrType, const char * name, ScriptCallback callback) = 0;
	virtual void RegisterOperator(unsigned ptrType, Operator op, ScriptCallback callback) = 0;

	virtual ScriptParam CreateMap() = 0;
	virtual void SetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value) = 0;
	virtual bool GetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value) = 0;
	virtual unsigned GetMapLength(ScriptParam mapref) = 0;

	virtual unsigned GetSpecialPtrType(SpecialPtrType type) = 0;

	void incDependencies() { dependencies++; }
	void decDependencies() { dependencies--; }
};

} // namespace Ingenuity

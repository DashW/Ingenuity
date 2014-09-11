#pragma once

#include <stdarg.h>
#include "ScriptLogger.h"
#include "ScriptParam.h"
#include <vector>

namespace Ingenuity {

class ScriptInterpreter;

struct ScriptCallback
{
	const char * name;
	void(*call)(ScriptInterpreter*);
	const char * help;

	ScriptCallback(const char * name, void(*callback)(ScriptInterpreter*), const char * help = "")
		: name(name), call(callback), help(help) {}
};

struct ScriptModule
{
	const char * name;
	const char * help;

	ScriptModule(const char * name, const char * help)
		: name(name), help(help) {}
};

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
	std::vector<ScriptCallback> callbacks;
	std::vector<ScriptModule> modules; // I'm not quite sure where I'm going with this...

	virtual ~ScriptInterpreter() {}

	virtual bool LoadScript(const char * data, unsigned dataSize, const char * filename) = 0;
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

	virtual void RegisterCallback(ScriptCallback & callback) = 0;
	virtual void RegisterCallback(const char * name, void(*callback)(ScriptInterpreter*), const char * help)
	{
		RegisterCallback(ScriptCallback(name, callback, help));
	}
	virtual void RegisterModule(ScriptModule & module) = 0;

	virtual ScriptParam CreateMap() = 0;
	virtual void SetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value) = 0;
	virtual bool GetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value) = 0;
	virtual unsigned GetMapLength(ScriptParam mapref) = 0;

	void incDependencies() { dependencies++; }
	void decDependencies() { dependencies--; }
};

} // namespace Ingenuity

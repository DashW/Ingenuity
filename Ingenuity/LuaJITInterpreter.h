#pragma once
#include "ScriptInterpreter.h"
#include "ScriptLogger.h"

struct lua_State;

class LuaJITInterpreter : public ScriptInterpreter
{
	virtual void FunctionCalled(const char * function) override;
	static const char * luaReferenceName;
	static const char * luaTempTablePrefix;

	static int LuaGC(lua_State * state);
	static int LuaPrint(lua_State * state);

	lua_State * state;

	void PushLuaParam(ScriptParam param);
	ScriptParam PopLuaParam();

	int PushLuaParams();
	void PopLuaParams(int num);

	int numTempTables;

public:
	LuaJITInterpreter(RealtimeApp * app);
	virtual ~LuaJITInterpreter();

	virtual bool LoadScript(const wchar_t * filename) override;

	virtual bool HasFunction(const char * name) override;

	virtual void RegisterCallback(ScriptCallback & callbacks) override;

	virtual void RegisterModule(ScriptModule & module) override;

	virtual void ThrowError(const char * error) override;

	virtual void RunCommand(const char * command) override;

	virtual void ReloadScript(const wchar_t * filename) override;

	virtual bool GetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value) override;

	static int Callback(lua_State *);
};


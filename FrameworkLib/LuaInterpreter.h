#pragma once
#include "ScriptInterpreter.h"
#include "ScriptLogger.h"

struct lua_State;

namespace Ingenuity {

class LuaInterpreter : public ScriptInterpreter
{
	virtual void FunctionCalled(const char * function) override;
	static const char * luaReferenceName;

	static int LuaGC(lua_State * state);
	static int LuaPrint(lua_State * state);

	static LuaInterpreter * GetInstance(lua_State * state);

	static int CreateVector(lua_State * state);
	static int CreateMatrix(lua_State * state);
	static int GetVector(lua_State * state);
	static int GetMatrix(lua_State * state);
	static int SetMatrix(lua_State * state);
	static int RotationMatrix(lua_State * state);
	static int AddMatrix(lua_State * state);
	static int MultiplyMatrix(lua_State * state);
	static int InverseMatrix(lua_State * state);
	static int TransposeMatrix(lua_State * state);

	static int CreateFloatArray(lua_State * state);
	static int SetFloatArray(lua_State * state);
	static int GetFloatArray(lua_State * state);

	void PushLuaParam(ScriptParam param);
	ScriptParam PopLuaParam();

	int PushLuaParams();
	void PopLuaParams(int num);

	bool CheckType(lua_State * state, unsigned typeIndex, int paramIndex);

	lua_State * state;
	std::vector<ScriptCallback> callbacks;
	std::vector<int> metatableRefs;
	std::vector<unsigned> structSizes;
	unsigned vector4type;
	unsigned matrix4type;
	unsigned floatArrayType;

public:
	LuaInterpreter(RealtimeApp * app);
	virtual ~LuaInterpreter();

	virtual bool LoadScript(const char * data, unsigned dataSize, const char * filename, const char * moduleName = 0) override;

	virtual bool HasFunction(const char * name) override;

	//virtual ScriptParam GetGlobal(const char * globalName) override;

	virtual void CallFunction(ScriptParam function) override;

	virtual unsigned RegisterPointerType(unsigned structSize = 0) override;

	virtual void RegisterCallback(const char * name, ScriptCallback callback) override;

	//virtual void RegisterModule(ScriptModule & module) override;

	virtual void RegisterMathObjects() override;

	virtual void RegisterMethod(unsigned ptrType, const char * name, ScriptCallback callback);

	virtual void RegisterOperator(unsigned ptrType, Operator op, ScriptCallback callback);

	virtual void ThrowError(const char * error) override;

	virtual void RunCommand(const char * command) override;

	virtual ScriptParam CreateMap() override;
	virtual void SetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value) override;
	virtual bool GetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value) override;
	virtual unsigned GetMapLength(ScriptParam mapref) override;

	virtual unsigned GetSpecialPtrType(SpecialPtrType type) override;

	static int Callback(lua_State *);
};

} // namespace Ingenuity

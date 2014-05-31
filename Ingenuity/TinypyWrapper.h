#pragma once

#include "ScriptLogger.h"
#include <map>

class GpuApi;
class AssetMgr;
class InterpreterLogger;

struct GpuSprite;
struct KeyState;
struct tp_vm;
union tp_obj;

struct TinypyParam
{
	enum ParamType {
		STRING, FLOAT, INT, POINTER, STATE, NONE
	};
	
	ParamType paramType;
	void* item;

	TinypyParam(ParamType t, void* i)
		:
		paramType(t),
		item(i)
	{}
};

class TinypyWrapper
{
public:
	TinypyWrapper();
	~TinypyWrapper();

	void Import(const char *filename, const char *modname = "__main__");
	void Call(const char *modname, const char *fncname, int argc, ...);
	TinypyParam Eval(char *expression);
	//TinypyParam GetState();

	void Error();
	bool IsInError();

	void RegisterModule();
	void SetState(GpuApi* gpu, AssetMgr *assets, KeyState* KeyState);

	ScriptLogger& GetLogger();

	static TinypyWrapper* GetInstance(tp_vm* ref);

private:
	static std::map<tp_vm*, TinypyWrapper&> instanceMap;

	tp_vm *tp;

	ScriptLogger logger;
	bool error;
};

void tpw_log(tp_vm* tp, const char* fmt, ...);
void tpw_err(tp_vm* tp);
bool tpw_in_err(tp_vm* tp);

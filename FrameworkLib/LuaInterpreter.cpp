#include "LuaInterpreter.h"
#include <lua.hpp>
#include <string>

namespace Ingenuity {

const char * LuaInterpreter::luaReferenceName = "_luaref";
const char * LuaInterpreter::luaTempTablePrefix = "_luatemptable_";

LuaInterpreter::LuaInterpreter(RealtimeApp * app) :
ScriptInterpreter(app), state(0), numTempTables(0)
{
	state = luaL_newstate();
	luaL_openlibs(state);

	lua_pushlightuserdata(state, this);
	lua_setglobal(state, luaReferenceName);

	lua_pushcfunction(state, LuaInterpreter::LuaPrint);
	lua_setglobal(state, "print");
}

LuaInterpreter::~LuaInterpreter()
{
	lua_close(state);
}

int LuaInterpreter::LuaGC(lua_State * state)
{
	IDeletingPtr ** deletingPtr = static_cast<IDeletingPtr**>(lua_touserdata(state, -1));
	(*deletingPtr)->DecRef();
	return 0;
}

int LuaInterpreter::LuaPrint(lua_State * state)
{
	int numParameters = lua_gettop(state);

	lua_getglobal(state, luaReferenceName);
	LuaInterpreter * instance = (LuaInterpreter *)lua_touserdata(state, -1);
	lua_pop(state, 1);

	std::string line = "";

	for(int i = 0; i < numParameters; i++)
	{
		const char * string = lua_tostring(state, -1);
		if(string)
		{
			line.insert(0, "   ");
			line.insert(0, string);
		}
		lua_pop(state, 1);
	}

	instance->GetLogger().Log("%s\n", line.c_str());

	return 0;
}

void LuaInterpreter::PushLuaParam(ScriptParam param)
{
	switch(param.type)
	{
	case ScriptParam::BOOL:
		lua_pushboolean(state, (int)param.nvalue);
		break;
	case ScriptParam::INT:
		lua_pushinteger(state, (lua_Integer)param.nvalue);
		break;
	case ScriptParam::FLOAT:
	case ScriptParam::DOUBLE:
		lua_pushnumber(state, (lua_Number)param.nvalue);
		break;
	case ScriptParam::STRING:
		lua_pushstring(state, param.svalue);
		break;
	case ScriptParam::MAPREF:
	{
		int mapIndex = int(param.nvalue);
		sprintf_s(tempTableName, "%s%d", luaTempTablePrefix, mapIndex);
		lua_getglobal(state, tempTableName);
		break;
	}
	case ScriptParam::POINTER:
	{
		IDeletingPtr ** ptr = static_cast<IDeletingPtr**>(lua_newuserdata(state, sizeof(IDeletingPtr*)));
		(*ptr) = param.pvalue;
		param.pvalue->IncRef();
		lua_newtable(state);
		lua_pushcfunction(state, &LuaInterpreter::LuaGC);
		lua_setfield(state, -2, "__gc");
		lua_setmetatable(state, -2);
		break;
	}
	case ScriptParam::FUNCTION:
	{
		int ref = int(param.nvalue);
		lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
		luaL_unref(state, LUA_REGISTRYINDEX, ref);
		break;
	}
	default:
		lua_pushnil(state);
	}
}

ScriptParam LuaInterpreter::PopLuaParam()
{
	ScriptParam::Type resultType = ScriptParam::NONE;
	double resultValue = 0.0;
	const char * stringValue = "";
	IDeletingPtr * ptrValue = 0;

	switch(lua_type(state, -1))
	{
	case LUA_TBOOLEAN:
		resultType = ScriptParam::BOOL;
		resultValue = lua_toboolean(state, -1) > 0 ? 1.0 : 0.0;
		break;
	case LUA_TNUMBER:
		resultType = ScriptParam::DOUBLE;
		resultValue = lua_tonumber(state, -1);
		break;
	case LUA_TSTRING:
		resultType = ScriptParam::STRING;
		stringValue = lua_tostring(state, -1);
		break;
	case LUA_TTABLE:
		resultType = ScriptParam::MAPREF;
		resultValue = double(numTempTables);
		sprintf_s(tempTableName, "%s%d", luaTempTablePrefix, numTempTables);
		numTempTables++;
		lua_setglobal(state, tempTableName);
		lua_pushnil(state);
		break;
	case LUA_TUSERDATA:
		resultType = ScriptParam::POINTER;
		ptrValue = *static_cast<IDeletingPtr**>(lua_touserdata(state, -1));
		break;
	case LUA_TFUNCTION:
		resultType = ScriptParam::FUNCTION;
		int ref = luaL_ref(state, LUA_REGISTRYINDEX);
		resultValue = double(ref);
		lua_pushnil(state);
		//ptrValue = new NonDeletingPtr(lua_tocfunction(state, -1), ScriptPtrType::ScriptFunction);
		break;
	}

	lua_pop(state, 1);

	switch(resultType)
	{
	case ScriptParam::STRING:
		return ScriptParam(resultType, stringValue);
	case ScriptParam::POINTER:
		return ScriptParam(ptrValue);
	default:
		return ScriptParam(resultType, resultValue);
	}
}

int LuaInterpreter::PushLuaParams()
{
	if(lua_checkstack(state, NumParams()))
	{
		for(int i = 0; i < NumParams(); i++)
		{
			PushLuaParam(GetParam(i));
		}
		int paramsPushed = NumParams();
		ClearParams();
		return paramsPushed;
	}
	else
	{
		ThrowError("Stack Overflow!");
		return 0;
	}
}

void LuaInterpreter::PopLuaParams(int num)
{
	SetNumParams(num);

	for(int i = 0; i < num; i++)
	{
		SetParam(i, PopLuaParam());
	}
}

bool LuaInterpreter::LoadScript(const char * data, unsigned dataSize, const char * filename)
{
	std::string filenameString("@");
	filenameString.append(filename);
	if((luaL_loadbuffer(state, data, dataSize, filenameString.c_str())
		|| lua_pcall(state, 0, LUA_MULTRET, 0)) != 0)
		//if(luaL_dostring(state, string) != 0)
	{
		const char * errormsg = lua_tostring(state, -1);
		GetLogger().Log("%s\n", errormsg);
		lua_pop(state, 1);
		SetError(true);
		SetInitialised();
		return false;
	}
	SetError(false);
	SetInitialised();
	return true;
}

bool LuaInterpreter::HasFunction(const char * name)
{
	lua_getglobal(state, name);

	bool hasFunction = lua_type(state, -1) == LUA_TFUNCTION;

	lua_pop(state, 1);

	return hasFunction;
}

void LuaInterpreter::FunctionCalled(const char * function)
{
	// Clear all the temp tables
	while(numTempTables > 0)
	{
		numTempTables--;
		sprintf_s(tempTableName, "%s%d", luaTempTablePrefix, numTempTables);
		lua_pushnil(state);
		lua_setglobal(state, tempTableName);
	}

	int luaStackBefore = lua_gettop(state);

	lua_getglobal(state, function);

	if(lua_pcall(state, PushLuaParams(), -1, 0) == 0)
	{
		int numResults = lua_gettop(state) - luaStackBefore;

		PopLuaParams(numResults);
	}
	else
	{
		const char * errormsg = lua_tostring(state, -1);
		GetLogger().Log("%s\n", errormsg);
		lua_pop(state, 1);
		SetError(true);
	}
}

void LuaInterpreter::CallFunction(ScriptParam function)
{
	if(function.type != ScriptParam::FUNCTION)
	{
		GetLogger().Log("C++ code attempted to call a script parameter that was not a function!\n");
		SetError(true);
		return;
	}

	int luaStackBefore = lua_gettop(state);

	int ref = int(function.nvalue);

	lua_rawgeti(state, LUA_REGISTRYINDEX, ref);

	if(lua_pcall(state, PushLuaParams(), -1, 0) == 0)
	{
		int numResults = lua_gettop(state) - luaStackBefore;

		PopLuaParams(numResults);
	}
	else
	{
		const char * errormsg = lua_tostring(state, -1);
		GetLogger().Log("%s\n", errormsg);
		lua_pop(state, 1);
		SetError(true);
	}

	luaL_unref(state, LUA_REGISTRYINDEX, ref);
}

void LuaInterpreter::RegisterCallback(ScriptCallback & callback)
{
	callbacks.push_back(callback);
	lua_pushnumber(state, (lua_Number)callbacks.size() - 1);
	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, &LuaInterpreter::Callback, 2);
	lua_setglobal(state, callback.name);
}

void LuaInterpreter::RegisterModule(ScriptModule & module)
{
	lua_pushstring(state, "RegisterModule Not Implemented Yet");
	lua_error(state);
}

void LuaInterpreter::ThrowError(const char * error)
{
	lua_Debug ar;
	if(lua_getstack(state, 1, &ar) == 1)
	{
		lua_getinfo(state, "nSl", &ar);
		int line = ar.currentline;

		char lineDigits[8];

		sprintf_s(lineDigits, "%d: ", line);

		std::string errorLine = ar.short_src;
		errorLine += ":";
		errorLine += lineDigits;
		errorLine += error;

		lua_pushstring(state, errorLine.c_str());
		lua_error(state);
	}
	else // Assume that the error happened out of context, e.g. another thread
	{
		GetLogger().Log("%s\n", error);
		SetError(true);
	}
}

void LuaInterpreter::RunCommand(const char * command)
{
	if(luaL_dostring(state, command) != 0)
	{
		const char * errormsg = lua_tostring(state, -1);
		GetLogger().Log("%s\n", errormsg);
		lua_pop(state, 1);
		SetError(true);
	}
}

ScriptParam LuaInterpreter::CreateMap()
{
	lua_newtable(state);
	return PopLuaParam();
}

void LuaInterpreter::SetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value)
{
	if(mapref.type != ScriptParam::MAPREF) return;

	int mapIndex = int(mapref.nvalue);
	sprintf_s(tempTableName, "%s%d", luaTempTablePrefix, mapIndex);

	lua_getglobal(state, tempTableName);
	if(lua_type(state, -1) != LUA_TTABLE) return;

	PushLuaParam(key);
	PushLuaParam(value);

	lua_settable(state, -3);

	lua_pop(state, 1);
}

bool LuaInterpreter::GetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value)
{
	if(mapref.type != ScriptParam::MAPREF) return false;

	int mapIndex = int(mapref.nvalue);
	sprintf_s(tempTableName, "%s%d", luaTempTablePrefix, mapIndex);

	lua_getglobal(state, tempTableName);
	if(lua_type(state, -1) != LUA_TTABLE)
	{
		return false;
	}
	PushLuaParam(key);
	if(lua_next(state, -2) != 0)
	{
		value = PopLuaParam();
		key = PopLuaParam();
		lua_pop(state, 1);
		return true;
	}
	else
	{
		return false;
	}
}

unsigned LuaInterpreter::GetMapLength(ScriptParam mapref)
{
	if(mapref.type != ScriptParam::MAPREF) return 0;

	int mapIndex = int(mapref.nvalue);
	sprintf_s(tempTableName, "%s%d", luaTempTablePrefix, mapIndex);

	lua_getglobal(state, tempTableName);
	if(lua_type(state, -1) != LUA_TTABLE)
	{
		return 0;
	}

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
	// Remove when the LuaJIT api is updated to Lua 5.2
	unsigned length = lua_objlen(state, -1);
#else
	unsigned length = luaL_len(state, -1);
#endif
	lua_pop(state, 1);
	return length;
}

int LuaInterpreter::Callback(lua_State * state)
{
	int methodIndex = (int)lua_tonumber(state, lua_upvalueindex(1));
	LuaInterpreter * instance = (LuaInterpreter *)lua_touserdata(state, lua_upvalueindex(2));

	int numParameters = lua_gettop(state);
	instance->PopLuaParams(numParameters);

	instance->callbacks[methodIndex].call(instance);

	return instance->PushLuaParams();
}

} // namespace Ingenuity

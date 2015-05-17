#include "LuaInterpreter.h"
#include <lua.hpp>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Ingenuity {

const char * LuaInterpreter::luaReferenceName = "_luaref";

LuaInterpreter::LuaInterpreter(RealtimeApp * app) :
	ScriptInterpreter(app), state(0), vector4type(0), matrix4type(0)
{
	state = luaL_newstate();
	luaL_openlibs(state);

	lua_pushlightuserdata(state, this);
	lua_setglobal(state, luaReferenceName);

	lua_pushcfunction(state, LuaInterpreter::LuaPrint);
	lua_setglobal(state, "print");

	metatableRefs.push_back(-1); // Unknown Type
	structSizes.push_back(0);
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
	case ScriptParam::DOUBLE:
		lua_pushnumber(state, (lua_Number)param.nvalue);
		break;
	case ScriptParam::STRING:
		lua_pushstring(state, param.svalue);
		break;
	case ScriptParam::MAPREF:
	{
		int ref = int(param.nvalue);
		lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
		luaL_unref(state, LUA_REGISTRYINDEX, ref);
		break;
	}
	case ScriptParam::POINTER:
	{
		if(param.pvalue->type == 0)
		{
			ThrowError("Parameter has unregistered type!");
			return;
		}
		unsigned structSize = structSizes[param.pvalue->type];
		if(structSize > 0)
		{
			void * src = param.pvalue->ptr;
			void * dest = lua_newuserdata(state, structSize);
			memcpy(dest, src, structSize);
		}
		else
		{
			IDeletingPtr ** ptr = static_cast<IDeletingPtr**>(lua_newuserdata(state, sizeof(IDeletingPtr*)));
			(*ptr) = ((IDeletingPtr*)param.pvalue);
			param.pvalue->IncRef();
		}
		lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[param.pvalue->type]);
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
		resultValue = double(luaL_ref(state, LUA_REGISTRYINDEX));
		lua_pushnil(state);
		break;
	case LUA_TUSERDATA:
	{
		resultType = ScriptParam::POINTER;
		lua_getmetatable(state, -1); // dangerous! lua_getmetatable could fail!
		lua_pushstring(state, "type");
		lua_gettable(state, -2);
		unsigned type = unsigned(lua_tonumber(state, -1));
		lua_pop(state, 2);
		unsigned structSize = structSizes[type];
		if(structSize == 0)
		{
			ptrValue = *static_cast<IDeletingPtr**>(lua_touserdata(state, -1));
			// Don't DecRef - this will be done on garbage collect
		}
		else
		{
			ptrValue = new BufferCopyPtr(lua_touserdata(state, -1), structSize, type);
		}
		break;
	}
	case LUA_TFUNCTION:
		//ptrValue = new NonDeletingPtr(lua_tocfunction(state, -1), ScriptPtrType::ScriptFunction);
		resultType = ScriptParam::FUNCTION;
		resultValue = double(luaL_ref(state, LUA_REGISTRYINDEX));
		lua_pushnil(state);
		break;
	}

	lua_pop(state, 1);

	switch(resultType)
	{
	case ScriptParam::STRING:
		return ScriptParam(resultType, stringValue);
	case ScriptParam::POINTER:
		// This performs an IncRef!
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

bool LuaInterpreter::CheckType(lua_State * state, unsigned typeIndex, int paramIndex)
{
	if(!lua_getmetatable(state, paramIndex)) return false;
	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[typeIndex]);
	bool isOfType = lua_rawequal(state, -1, -2) > 0;
	lua_pop(state, 2);
	return isOfType;
}

bool LuaInterpreter::LoadScript(const char * data, unsigned dataSize, const char * filename, const char * moduleName)
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
	if(moduleName && strlen(moduleName) > 0)
	{
		lua_setglobal(state, moduleName);
	}
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

//ScriptParam LuaInterpreter::GetGlobal(const char * globalName)
//{
//	lua_getglobal(state, globalName);
//	return PopLuaParam();
//}

void LuaInterpreter::FunctionCalled(const char * function)
{
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

unsigned LuaInterpreter::RegisterPointerType(unsigned structSize)
{
	lua_newtable(state);  /* create metatable */
	lua_pushvalue(state, -1);
	lua_pushinteger(state, metatableRefs.size());
	lua_setfield(state, -2, "type");
	if(structSize == 0)
	{
		lua_pushcfunction(state, &LuaInterpreter::LuaGC);
		lua_setfield(state, -2, "__gc");
	}
	metatableRefs.push_back(luaL_ref(state, LUA_REGISTRYINDEX));
	structSizes.push_back(structSize);
	return metatableRefs.size() - 1;
}

void LuaInterpreter::RegisterCallback(const char * name, ScriptCallback callback)
{
	callbacks.push_back(callback);
	lua_pushnumber(state, (lua_Number)callbacks.size() - 1);
	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, &LuaInterpreter::Callback, 2);
	lua_setglobal(state, name);
}

//void LuaInterpreter::RegisterModule(ScriptModule & module)
//{
//	lua_pushstring(state, "RegisterModule Not Implemented Yet");
//	lua_error(state);
//}

static int vec4Set(lua_State * state)
{
	if(luaL_checkudata(state, 1, "vector4") == NULL) luaL_error(state, "expected vector4");
	//glm::vec4 * vector4 = (glm::vec4*) lua_touserdata(state, 1);
	glm::vec4 * vector4 = *((glm::vec4**) lua_touserdata(state, 1));
	const char* i = luaL_checkstring(state, 2);
	double t = luaL_checknumber(state, 3);
	switch(*i) {		/* lazy! */
	case '1': case 'x': case 'r': vector4->x = float(t); break;
	case '2': case 'y': case 'g': vector4->y = float(t); break;
	case '3': case 'z': case 'b': vector4->z = float(t); break;
	case '4': case 'w': case 'a': vector4->w = float(t); break;
	default: break;
	}
	return 1;
}

static int vec4String(lua_State * state)
{
	if(luaL_checkudata(state, 1, "vector4") == NULL) luaL_error(state, "expected vector4");
	//glm::vec4 * vector4 = (glm::vec4*) lua_touserdata(state, 1);
	glm::vec4 * vector4 = *((glm::vec4**) lua_touserdata(state, 1));
	char s[64];
	sprintf_s(s, "(%2.2f,%2.2f,%2.2f,%2.2f)", vector4->x, vector4->y, vector4->z, vector4->w);
	lua_pushstring(state, s);
	return 1;
}

LuaInterpreter * LuaInterpreter::GetInstance(lua_State * state)
{
	lua_getglobal(state, LuaInterpreter::luaReferenceName);
	LuaInterpreter * interpreter = (LuaInterpreter*)lua_touserdata(state, -1);
	lua_pop(state, 1);
	return interpreter;
}

int LuaInterpreter::CreateVector(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	glm::vec4 * vector4 = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
	vector4->x = float(luaL_optnumber(state, 1, 0));
	vector4->y = float(luaL_optnumber(state, 2, 0));
	vector4->z = float(luaL_optnumber(state, 3, 0));
	vector4->w = float(luaL_optnumber(state, 4, 0));
	lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
	lua_setmetatable(state, -2);
	return 1;
}
int LuaInterpreter::CreateMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	glm::mat4 * matrix4 = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
	float initialValue = 1.0f;
	if(lua_type(state, 2) == LUA_TNUMBER) initialValue = float(lua_tonumber(state, 2));
	*matrix4 = glm::mat4(initialValue);
	lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
	lua_setmetatable(state, -2);
	return 1;
}
int LuaInterpreter::GetVector(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	if(!lua_getmetatable(state, 1)) luaL_error(state, "expected vector4");
	lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
	if(!lua_rawequal(state, -1, -2)) luaL_error(state, "expected vector4");
	lua_pop(state, 2);
	glm::vec4 * vector4 = (glm::vec4*) lua_touserdata(state, 1);
	const char* i = luaL_checkstring(state, 2);
	switch(*i) {		/* lazy! */
	case '0': case 'x': case 'r': lua_pushnumber(state, vector4->x); break;
	case '1': case 'y': case 'g': lua_pushnumber(state, vector4->y); break;
	case '2': case 'z': case 'b': lua_pushnumber(state, vector4->z); break;
	case '3': case 'w': case 'a': lua_pushnumber(state, vector4->w); break;
	default: lua_pushnil(state); break;
	}
	return 1;
}
int LuaInterpreter::GetMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	if(interpreter->CheckType(state, interpreter->matrix4type, 1))
	{
		const glm::mat4 * matrix4 = (glm::mat4*) lua_touserdata(state, 1);
		glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
		const char* i = luaL_checkstring(state, 2);
		switch(*i)
		{
		case '0': *result = (*matrix4)[0]; break;
		case '1': *result = (*matrix4)[1]; break;
		case '2': *result = (*matrix4)[2]; break;
		case '3': *result = (*matrix4)[3]; break;
		default: luaL_error(state, "matrix index '%c' out of range", *i); return 0;
		}
		lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
		lua_setmetatable(state, -2);
	}
	return 1;
}
int LuaInterpreter::SetMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	if(interpreter->CheckType(state, interpreter->matrix4type, 1))
	{
		glm::mat4 * matrix4 = (glm::mat4*) lua_touserdata(state, 1);
		const char* i = luaL_checkstring(state, 2);

		if(interpreter->CheckType(state, interpreter->vector4type, 3))
		{
			const glm::vec4 * vector4 = (glm::vec4*) lua_touserdata(state, 3);
			switch(*i)
			{
			case '0': (*matrix4)[0] = *vector4; break;
			case '1': (*matrix4)[1] = *vector4; break;
			case '2': (*matrix4)[2] = *vector4; break;
			case '3': (*matrix4)[3] = *vector4; break;
			default: luaL_error(state, "matrix index out of range"); return 0;
			}
		}
		else
		{
			luaL_error(state, "expected vector4");
		}
	}
	return 1;
}
int LuaInterpreter::RotationMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	glm::mat4 * matrix4 = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
	lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
	lua_setmetatable(state, -2);
	(*matrix4) = glm::eulerAngleYXZ(float(luaL_optnumber(state, 2, 0)), float(luaL_optnumber(state, 1, 0)), float(luaL_optnumber(state, 3, 0)));
	return 1;
}
int LuaInterpreter::AddMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);

	if(interpreter->CheckType(state, interpreter->matrix4type, 1))
	{
		const glm::mat4 * matrix4left = (glm::mat4*) lua_touserdata(state, 1);

		if(interpreter->CheckType(state, interpreter->matrix4type, 2))
		{
			const glm::mat4 * matrix4right = (glm::mat4*) lua_touserdata(state, 2);
			glm::mat4 * result = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
			*result = (*matrix4left) + (*matrix4right);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
			lua_setmetatable(state, -2);
		}
		else if(lua_isnumber(state, 2))
		{
			double numberRight = lua_tonumber(state, 2);
			glm::mat4 * result = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
			*result = (*matrix4left) + float(numberRight);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
			lua_setmetatable(state, -2);
		}
		else
		{
			luaL_error(state, "expected matrix/number for parameter 2");
		}
	}
	else if(interpreter->CheckType(state, interpreter->vector4type, 1))
	{
		const glm::vec4 * vector4left = (glm::vec4*) lua_touserdata(state, 1);

		if(interpreter->CheckType(state, interpreter->vector4type, 2))
		{
			const glm::vec4 * vector4right = (glm::vec4*) lua_touserdata(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) + (*vector4right);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else if(lua_isnumber(state, 2))
		{
			double numberRight = lua_tonumber(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) + float(numberRight);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else
		{
			luaL_error(state, "expected vector/number for parameter 2");
		}
	}
	else
	{
		luaL_error(state, "expected vector/matrix for parameter 1");
	}

	return 1;
}
int LuaInterpreter::SubMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);

	if(interpreter->CheckType(state, interpreter->vector4type, 1))
	{
		const glm::vec4 * vector4left = (glm::vec4*) lua_touserdata(state, 1);

		if(interpreter->CheckType(state, interpreter->vector4type, 2))
		{
			const glm::vec4 * vector4right = (glm::vec4*) lua_touserdata(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) - (*vector4right);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else if(lua_isnumber(state, 2))
		{
			double numberRight = lua_tonumber(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) - float(numberRight);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else
		{
			luaL_error(state, "expected vector/number for parameter 2");
		}
	}
	else
	{
		luaL_error(state, "expected vector/matrix for parameter 1");
	}

	return 1;
}
int LuaInterpreter::MultiplyMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);

	if(interpreter->CheckType(state, interpreter->matrix4type, 1))
	{
		const glm::mat4 * matrix4left = (glm::mat4*) lua_touserdata(state, 1);

		if(interpreter->CheckType(state, interpreter->matrix4type, 2))
		{
			const glm::mat4 * matrix4right = (glm::mat4*) lua_touserdata(state, 2);
			glm::mat4 * result = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
			*result = (*matrix4left) * (*matrix4right);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
			lua_setmetatable(state, -2);
		}
		else if(interpreter->CheckType(state, interpreter->vector4type, 2))
		{
			const glm::vec4 * vector4right = (glm::vec4*) lua_touserdata(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*matrix4left) * (*vector4right);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else if(lua_isnumber(state, 2))
		{
			double numberRight = lua_tonumber(state, 2);
			glm::mat4 * result = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
			*result = (*matrix4left) * float(numberRight);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
			lua_setmetatable(state, -2);
		}
		else
		{
			luaL_error(state, "expected vector/matrix/number for parameter 2");
		}
	}
	else if(interpreter->CheckType(state, interpreter->vector4type, 1))
	{
		const glm::vec4 * vector4left = (glm::vec4*) lua_touserdata(state, 1);

		if(interpreter->CheckType(state, interpreter->matrix4type, 2))
		{
			const glm::mat4 * matrix4right = (glm::mat4*) lua_touserdata(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) * (*matrix4right);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else if(interpreter->CheckType(state, interpreter->vector4type, 2))
		{
			const glm::vec4 * vector4right = (glm::vec4*) lua_touserdata(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) * (*vector4right);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else if(lua_isnumber(state, 2))
		{
			double numberRight = lua_tonumber(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) * float(numberRight);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else
		{
			luaL_error(state, "expected vector/matrix/number for parameter 2");
		}
	}
	else
	{
		luaL_error(state, "expected vector/matrix for parameter 1");
	}

	return 1;
}
int LuaInterpreter::DivideMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);

	if(interpreter->CheckType(state, interpreter->matrix4type, 1))
	{
		const glm::mat4 * matrix4left = (glm::mat4*) lua_touserdata(state, 1);

		if(lua_isnumber(state, 2))
		{
			double numberRight = lua_tonumber(state, 2);
			glm::mat4 * result = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
			*result = (*matrix4left) / float(numberRight);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
			lua_setmetatable(state, -2);
		}
		else
		{
			luaL_error(state, "expected vector/matrix/number for parameter 2");
		}
	}
	else if(interpreter->CheckType(state, interpreter->vector4type, 1))
	{
		const glm::vec4 * vector4left = (glm::vec4*) lua_touserdata(state, 1);

		if(lua_isnumber(state, 2))
		{
			double numberRight = lua_tonumber(state, 2);
			glm::vec4 * result = (glm::vec4*) lua_newuserdata(state, sizeof(glm::vec4));
			*result = (*vector4left) / float(numberRight);
			lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->vector4type]);
			lua_setmetatable(state, -2);
		}
		else
		{
			luaL_error(state, "expected vector/matrix/number for parameter 2");
		}
	}
	else
	{
		luaL_error(state, "expected vector/matrix for parameter 1");
	}

	return 1;
}
int LuaInterpreter::InverseMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	
	if(!interpreter->CheckType(state, interpreter->matrix4type, 1)) luaL_error(state, "expected matrix4");

	const glm::mat4 * matrix4 = (glm::mat4*) lua_touserdata(state, 1);
	glm::mat4 * result = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
	*result = glm::inverse(*matrix4);
	lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
	lua_setmetatable(state, -2);

	return 1;
}

int LuaInterpreter::TransposeMatrix(lua_State * state)
{
	LuaInterpreter * interpreter = GetInstance(state);
	
	glm::vec4 translation;
	bool preserveTranslation = false;
	if(lua_type(state, 2) == LUA_TBOOLEAN)
	{
		preserveTranslation = lua_toboolean(state, 2) > 0;
	}

	if(!interpreter->CheckType(state, interpreter->matrix4type, 1)) luaL_error(state, "expected matrix4");
	
	const glm::mat4 * matrix4 = (glm::mat4*) lua_touserdata(state, 1);
	glm::mat4 * result = (glm::mat4*) lua_newuserdata(state, sizeof(glm::mat4));
	if(preserveTranslation)
	{
		translation = (*matrix4)[3];
	}
	*result = glm::transpose(*matrix4);
	if(preserveTranslation)
	{
		(*result)[3] = translation;
	}
	lua_rawgeti(state, LUA_REGISTRYINDEX, interpreter->metatableRefs[interpreter->matrix4type]);
	lua_setmetatable(state, -2);
	
	return 1;
}

void LuaInterpreter::RegisterMathObjects()
{
	vector4type = RegisterPointerType(sizeof(glm::vec4));
	matrix4type = RegisterPointerType(sizeof(glm::mat4));

	lua_register(state, "CreateVector", CreateVector);
	lua_register(state, "CreateMatrix", CreateMatrix);
	lua_register(state, "GetVector", GetVector);
	lua_register(state, "GetMatrix", GetMatrix);
	lua_register(state, "SetMatrix", SetMatrix);
	lua_register(state, "RotMatrix", RotationMatrix);
	lua_register(state, "AddMatrix", AddMatrix);
	lua_register(state, "SubMatrix", SubMatrix);
	lua_register(state, "MulMatrix", MultiplyMatrix);
	lua_register(state, "DivMatrix", DivideMatrix);
	lua_register(state, "InvMatrix", InverseMatrix);
	lua_register(state, "TraMatrix", TransposeMatrix);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::vector4type]);
	lua_pushstring(state, "__index");
	lua_pushcfunction(state, GetVector);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::matrix4type]);
	lua_pushstring(state, "__index");
	lua_pushcfunction(state, GetMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::matrix4type]);
	lua_pushstring(state, "__newindex");
	lua_pushcfunction(state, SetMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::vector4type]);
	lua_pushstring(state, "__mul");
	lua_pushcfunction(state, MultiplyMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::matrix4type]);
	lua_pushstring(state, "__mul");
	lua_pushcfunction(state, MultiplyMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::vector4type]);
	lua_pushstring(state, "__add");
	lua_pushcfunction(state, AddMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::matrix4type]);
	lua_pushstring(state, "__add");
	lua_pushcfunction(state, AddMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::vector4type]);
	lua_pushstring(state, "__sub");
	lua_pushcfunction(state, SubMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::vector4type]);
	lua_pushstring(state, "__div");
	lua_pushcfunction(state, DivideMatrix);
	lua_settable(state, -3);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[LuaInterpreter::matrix4type]);
	lua_pushstring(state, "__div");
	lua_pushcfunction(state, DivideMatrix);
	lua_settable(state, -3);
}

void LuaInterpreter::RegisterMethod(unsigned ptrType, const char * name, ScriptCallback callback)
{
	callbacks.push_back(callback);

	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableRefs[ptrType]);

	lua_pushstring(state, name);

	lua_pushnumber(state, (lua_Number)callbacks.size() - 1);
	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, &LuaInterpreter::Callback, 2);

	lua_settable(state, -3);
}

void LuaInterpreter::RegisterOperator(unsigned ptrType, Operator op, ScriptCallback callback)
{
	const char* metaFuncName = 0;

	switch(op)
	{
	case Call:
		metaFuncName = "__call";
		break;
	case Add:
		metaFuncName = "__add";
		break;
	case Sub:
		metaFuncName = "__sub";
		break;
	case Mul:
		metaFuncName = "__mul";
		break;
	case Div:
		metaFuncName = "__div";
		break;
	case ToString:
		metaFuncName = "__tostring";
		break;
	case IndexGet:
		metaFuncName = "__index";
		break;
	case IndexSet:
		metaFuncName = "__newindex";
		break;
	case Length:
		metaFuncName = "__len";
		break;
	}

	if(!metaFuncName) return;

	RegisterMethod(ptrType, metaFuncName, callback);
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
		//SetError(true);
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

	int ref = int(mapref.nvalue);
	lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
	if(lua_type(state, -1) != LUA_TTABLE) return;

	PushLuaParam(key);
	PushLuaParam(value);

	lua_settable(state, -3);

	lua_pop(state, 1);
}

bool LuaInterpreter::GetMapNext(ScriptParam mapref, ScriptParam & key, ScriptParam & value)
{
	if(mapref.type != ScriptParam::MAPREF) return false;

	int ref = int(mapref.nvalue);
	lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
	if(lua_type(state, -1) != LUA_TTABLE) return false;

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

	int ref = int(mapref.nvalue);
	lua_rawgeti(state, LUA_REGISTRYINDEX, ref);

	if(lua_type(state, -1) != LUA_TTABLE) return 0;

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_APP
	// Remove when the LuaJIT api is updated to Lua 5.2
	unsigned length = lua_objlen(state, -1);
#else
	unsigned length = luaL_len(state, -1);
#endif
	lua_pop(state, 1);
	return length;
}

unsigned LuaInterpreter::GetSpecialPtrType(SpecialPtrType type)
{
	switch(type)
	{
	case TypeVector4:
		return vector4type;
	case TypeMatrix4:
		return matrix4type;
	default:
		return 0;
	}
}

int LuaInterpreter::Callback(lua_State * state)
{
	int methodIndex = (int)lua_tonumber(state, lua_upvalueindex(1));
	LuaInterpreter * instance = (LuaInterpreter *)lua_touserdata(state, lua_upvalueindex(2));

	int numParameters = lua_gettop(state);
	instance->PopLuaParams(numParameters);

	instance->callbacks[methodIndex](instance);

	return instance->PushLuaParams();
}

} // namespace Ingenuity

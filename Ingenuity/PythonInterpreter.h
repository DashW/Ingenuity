#pragma once

//#include "ScriptInterpreter.h"
//
//struct _object;
//typedef _object PyObject;
//
//class PythonInterpreter : public ScriptInterpreter
//{
//public:
//	PythonInterpreter();
//	virtual ~PythonInterpreter();
//
//	virtual bool LoadScript(const wchar_t * filename) override;
//
//protected:
//	virtual ScriptParam FunctionCalled(const char* functionName) override;
//
//	PyObject* module;
//};
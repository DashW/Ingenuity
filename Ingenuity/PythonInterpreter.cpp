//#include "PythonInterpreter.h"
//
//#include <Python.h>
//#include <string>
//
//PythonInterpreter::PythonInterpreter()
//	: module(0)
//{
//	Py_Initialize();
//}
//
//PythonInterpreter::~PythonInterpreter()
//{
//	if(module) Py_DecRef(module);
//}
//
//bool PythonInterpreter::LoadScript(const wchar_t * buffer)
//{
//	std::wstring wideString = buffer;
//	std::string shortString(wideString.begin(),wideString.end());
//	PyObject * name = PyUnicode_FromString(shortString.c_str());
//	module = PyImport_Import(name);
//	Py_DecRef(name);
//	return (module != Py_None);
//}
//
//ScriptParam PythonInterpreter::FunctionCalled(const char * functionName)
//{
//	// pDict is a borrowed reference 
//    PyObject * pDict = PyModule_GetDict(module);
//
//    // pFunc is also a borrowed reference 
//    PyObject * pFunc = PyDict_GetItemString(pDict, functionName);
//
//    if (PyCallable_Check(pFunc)) 
//    {
//		int numParams = NumParams();
//		if(numParams > 0)
//		{
//			PyObject *args = 0;
//			args = PyTuple_New(numParams);
//			for(int i = 0; i < numParams; ++i)
//			{
//				PyObject *value = 0;
//				ScriptParam &param = GetParam(i);
//				switch(param.type)
//				{
//				case ScriptParam::NONE:
//					value = Py_None;
//
//				}
//				PyTuple_SetItem(args, i, value);
//			}
//			PyObject_CallObject(pFunc, args);
//			Py_DecRef(args);
//		}
//		else
//		{
//			PyObject_CallObject(pFunc, NULL);
//		}
//    } else 
//    {
//        PyErr_Print();
//    }
//
//	return ScriptParam(ScriptParam::NONE, ScriptParam::paramData());
//}

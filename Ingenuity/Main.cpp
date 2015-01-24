//                                                 //
//    --------- Ingenuity Game Engine ---------    //
//                                                 //
//    Created by Richard Copperwaite 2012-2014     //
//                                                 //

#include <RealtimeApp.h>

#include "IngenuityHelper.h"
#include "ScriptConsole.h"
#include "ScriptInterpreter.h"
#include "ScriptCallbacks.h"
#include "ScriptTypes.h"
#include "GpuScriptCallbacks.h"
//#include "PythonInterpreter.h"
#include "InputState.h"
#include "LuaInterpreter.h"
#include "AudioApi.h"
#include <tinyxml2.h>
#include <string>

#define _X86_
#include <debugapi.h>
#undef _X86_

namespace Ingenuity {

const wchar_t * const CONFIG_FILE_NAME = L"config.xml";

class Engine : public RealtimeApp
{
	enum InterpreterType
	{
		Lua,
		Python
	};

	ScriptInterpreter * interpreter = 0;
	ScriptConsole * console = 0;
	InterpreterType interpreterType;

	bool beginCalled = false;
	bool failedOnInit = false;
	bool showConsole = false;
	bool audioPaused = false;

	Files::Directory * dataDirectory = 0;
	Files::Directory * projectDirectory = 0;

	std::wstring projectName;
	std::wstring scriptPath;

	//ScriptParam updateFunction;
	//ScriptParam drawFunction;

	struct ScriptResponse : public Files::Response
	{
		Engine * parent;
		std::string filename;
		bool reloading;

		ScriptResponse(Engine * i, std::wstring filename, bool reloading) : parent(i), reloading(reloading)
		{
			this->filename.assign(filename.begin(), filename.end());
		}

		virtual void Respond() override
		{
			closeOnComplete = true; deleteOnComplete = true;

			if(buffer)
			{
				parent->interpreter->LoadScript(buffer, bufferLength, filename.c_str());

				// Need to delay the Begin() call until we've loaded all dependencies...

				if(reloading && !parent->failedOnInit)
				{
					if(parent->interpreter->HasFunction("Reload"))
					{
						parent->interpreter->ClearParams();
						parent->interpreter->CallFunction("Reload");
					}

					parent->showConsole = false;
				}
			}
			else
			{
				OutputDebugString(L"Could not initialise application, failed to find script file!\n");
				if(IsDebuggerPresent())
				{
					__debugbreak();
				}
			}
		}
	};

	void InitialiseScript()
	{
		if(projectName.size() > 0)
		{
			std::wstring projectDirectoryPath = L"Projects//";
			projectDirectoryPath += projectName + L"//";
			projectDirectory = files->GetSubDirectory(dataDirectory, projectDirectoryPath.data());
			ScriptCallbacks::SetSubDirectory(projectDirectory);

			std::wstring scriptExtension = L".lua";
			if(interpreterType == Python) scriptExtension = L".py";
			scriptPath = projectName + scriptExtension;

			files->OpenAndRead(projectDirectory, scriptPath.c_str(), new ScriptResponse(this, scriptPath, false));
		}
	}

	struct ConfigResponse : public Files::Response
	{
		Engine * parent;

		ConfigResponse(Engine * parent) : parent(parent) {}

		virtual void Respond() override
		{
			closeOnComplete = true; deleteOnComplete = true;

			if(buffer)
			{
				tinyxml2::XMLDocument configDocument;
				if(configDocument.Parse(buffer, bufferLength) == tinyxml2::XML_NO_ERROR)
				{
					tinyxml2::XMLElement * rootConfigElement = configDocument.RootElement();
					if(strcmp(rootConfigElement->Name(), "config") == 0)
					{
						tinyxml2::XMLElement * configElement = rootConfigElement->FirstChildElement();
						while(configElement)
						{
							if(strcmp(configElement->Name(), "project") == 0)
							{
								tinyxml2::XMLText * projectText = configElement->FirstChild()->ToText();
								if(projectText)
								{
									std::string projectNameShort = projectText->Value();
									parent->projectName.assign(projectNameShort.begin(), projectNameShort.end());
								}
							}
							if(strcmp(configElement->Name(), "language") == 0)
							{
								tinyxml2::XMLText * languageText = configElement->FirstChild()->ToText();
								if(languageText)
								{
									const char * language = languageText->Value();
									if(strcmp(language, "lua") == 0) parent->interpreterType = Lua;
									if(strcmp(language, "python") == 0) parent->interpreterType = Python;
								}
							}

							// It would be great to add all these config properties to a
							// config object in scripting-land...

							configElement = configElement->NextSiblingElement();
						}
					}
				}
			}

			parent->InitialiseScript();
		}
	};

	virtual void Begin() override
	{
		interpreterType = Lua;

		// Create the interpreter here.
		// Could be Lua, Python, CINT, Scheme, whatever...
		if(interpreterType == Lua) interpreter = new LuaInterpreter(this);
		if(!interpreter) interpreter = new LuaInterpreter(this);

		interpreter->RegisterMathObjects();

		ScriptTypes::RegisterWith(interpreter);
		ScriptCallbacks::RegisterWith(interpreter);
		GpuScriptCallbacks::RegisterWith(interpreter);

		interpreter->GetLogger().SetLogFile(L"log.txt");
		console = new ScriptConsole(interpreter, gpu);

		//gpu->SetClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		//IngenuityHelper::RequestDataDirectory(files);
	}

	virtual void Update(float delta) override
	{
		if(!interpreter->IsInitialised())
		{
			if(!dataDirectory)
			{
				if(IngenuityHelper::IsRequestCompleted())
				{
					dataDirectory = IngenuityHelper::GetDataDirectory();
					if(dataDirectory)
					{
						files->OpenAndRead(dataDirectory, CONFIG_FILE_NAME, new ConfigResponse(this));
					}
					else
					{
						IngenuityHelper::RequestDataDirectory(files);
					}
				}
			}
			return;
		}

		if(!beginCalled && !interpreter->IsInError())
		{
			interpreter->ClearParams();
			interpreter->CallFunction("Begin");

			if(interpreter->IsInError())
			{
				failedOnInit = true;
				showConsole = true;
			}
			else
			{
				beginCalled = true;
			}
		}

		KeyState & keyboard = input->GetKeyState();
		MouseState & mouse = input->GetMouseState();

		if(!showConsole)
		{
			//if(interpreter->HasFunction("KeyDown"))
			//{
			//	for(unsigned i = 0; i < MAX_KEYS; ++i)
			//	{
			//		if(keyboard.downKeys[i])
			//		{
			//			interpreter->ClearParams();
			//			interpreter->PushParam(ScriptParam(ScriptParam::INT, (int)i));
			//			interpreter->CallFunction("KeyDown");
			//		}
			//	}
			//}
			//if(interpreter->HasFunction("KeyUp"))
			//{
			//	for(unsigned i = 0; i < MAX_KEYS; ++i)
			//	{
			//		if(keyboard.upKeys[i])
			//		{
			//			interpreter->ClearParams();
			//			interpreter->PushParam(ScriptParam(ScriptParam::INT, (int)i));
			//			interpreter->CallFunction("KeyUp");
			//		}
			//	}
			//}
			//if(interpreter->HasFunction("MouseMoved"))
			//{
			//	if(mouse.dX != 0.0f && mouse.dY != 0.0f)
			//	{
			//		interpreter->ClearParams();
			//		interpreter->PushParam(ScriptParam(ScriptParam::FLOAT, mouse.dX));
			//		interpreter->PushParam(ScriptParam(ScriptParam::FLOAT, mouse.dY));
			//		interpreter->CallFunction("MouseMoved");
			//	}
			//}
		}
		else
		{
			console->ProcessInput(keyboard);
		}

		if(keyboard.downKeys[0x3e]) // f4
		{
			showConsole = !showConsole;
		}
		if(keyboard.downKeys[0x3f]) // f5
		{
			files->OpenAndRead(projectDirectory, scriptPath.c_str(), new ScriptResponse(this, scriptPath, true));
		}

		if(!showConsole && !interpreter->IsInError())
		{
			interpreter->ClearParams();
			interpreter->PushParam(ScriptParam(ScriptParam::DOUBLE, double(delta)));
			interpreter->CallFunction("Update");
		}

		if(interpreter->IsInError()) showConsole = true;

		if(showConsole != audioPaused)
		{
			audio->Pause();
			audioPaused = !audioPaused;
		}

		if(keyboard.downKeys[1]) //esc
		{
			running = false;
		}
	}

	virtual void Draw() override
	{
		if(interpreter->IsInitialised() && !interpreter->IsInError())
		{
			interpreter->ClearParams();
			interpreter->CallFunction("Draw");
		}

		if(showConsole)
		{
			gpu->Draw(console);
		}
	}

	virtual void End() override
	{
		if(interpreter && !interpreter->IsInError())
		{
			interpreter->ClearParams();
			interpreter->CallFunction("End");
		}

		delete console;
		delete interpreter;

		IngenuityHelper::DeleteDataDirectory();
	}

};

} // namespace Ingenuity

MAIN_WITH(Ingenuity::Engine)

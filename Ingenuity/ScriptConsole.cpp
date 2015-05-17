#include "ScriptConsole.h"
#include <ScriptInterpreter.h>
#include <ScriptLogger.h>
#include <GpuApi.h>
#include <InputState.h>

#define CHAR_TO_WCHAR_T(IN,OUT) \
	std::string tmp1(IN);\
	std::wstring tmp2(tmp1.begin(), tmp1.end());\
	OUT = tmp2.c_str()

namespace Ingenuity {

ScriptConsole::ScriptConsole(ScriptInterpreter * interpreter, Gpu::Api * gpu)
	: interpreter(interpreter)
	, prevInputIndex(0)
{
	// Source Code Pro?	BPmono?
	displayFont = gpu->CreateGpuFont(20, L"Courier New", Gpu::FontStyle_Bold);
	shadowFont = gpu->CreateGpuFont(20, L"Courier New", Gpu::FontStyle_Bold);
	if(displayFont && shadowFont)
	{
		displayFont->color.r = 1.0f;
		displayFont->color.g = 0.27f;
		shadowFont->color.a = 0.6f;
	}
}

ScriptConsole::~ScriptConsole()
{
	if(displayFont) delete displayFont;
	if(shadowFont) delete shadowFont;
}

void ScriptConsole::ProcessInput(KeyState &keystate)
{
	if(keystate.downKeys[0x48]) // up
	{
		if(prevInputIndex < previousInputs.size())
		{
			prevInputIndex++;
			inputString = previousInputs[previousInputs.size() - prevInputIndex];
		}
	}
	if(keystate.downKeys[0x50]) // down
	{
		if(prevInputIndex > 0)
		{
			prevInputIndex--;
			inputString = (prevInputIndex > 0 ? previousInputs[previousInputs.size() - prevInputIndex] : "");
		}
	}
	for(size_t i = 0; i < keystate.text.length(); i++)
	{
		if(keystate.text[i] == '\b') // backspace
		{
			if(inputString.length() > 0)
			{
				inputString.erase(inputString.length() - 1, 1);
			}
		}
		else if(keystate.text[i] == '\r') // enter
		{
			if(previousInputs.size() >= 20) 
			{
				previousInputs.erase(previousInputs.begin());
			}
			previousInputs.push_back(inputString);
			prevInputIndex = 0;
			interpreter->GetLogger().Log("%s\n", inputString.c_str());
			interpreter->RunCommand(inputString.c_str());
			inputString.erase();
		}
		else if(keystate.text[i] == 0x7F) // control-backspace
		{
			if(inputString.length() > 0)
			{
				inputString.erase(inputString.length() - 1, 1);
			}
			while(inputString.length() > 0 && isalnum(inputString.back()))
			{
				inputString.erase(inputString.length() - 1, 1);
			}
		}
		else
		{
			inputString += keystate.text[i];
		}
	}
}

void ScriptConsole::BeDrawn(Gpu::Api * gpu, Gpu::DrawSurface * surface)
{
	if(!displayFont || !shadowFont) return;
	ScriptLogger& logger = interpreter->GetLogger();
	int logLines = logger.GetNumLines();
	const wchar_t* out;
	for(int i = 0; i < logLines; i++)
	{
		CHAR_TO_WCHAR_T(logger.GetLine(i), out);
		gpu->DrawGpuText(shadowFont, out, 11.0f, (20.0f * float(i)) + 1.0f, false, surface);
		gpu->DrawGpuText(displayFont, out, 10.0f, 20.0f * float(i), false, surface);
	}
	std::string prompt(">>> ");
	prompt += inputString;
	prompt += "_";
	CHAR_TO_WCHAR_T(prompt.c_str(), out);
	gpu->DrawGpuText(shadowFont, out, 11.0f, (20.0f * float(logLines)) + 1.0f, false, surface);
	gpu->DrawGpuText(displayFont, out, 10.0f, 20.0f * float(logLines), false, surface);
}

} // namespace Ingenuity

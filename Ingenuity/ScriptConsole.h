#pragma once

#include <GpuApi.h>
#include <string>

namespace Ingenuity {

struct KeyState;
class ScriptInterpreter;

class ScriptConsole : public Gpu::Drawable
{
public:
	ScriptConsole(ScriptInterpreter * interpreter, Gpu::Api * gpu);
	~ScriptConsole();

	void ProcessInput(KeyState & keyState);
	virtual void BeDrawn(Gpu::Api * gpu, Gpu::DrawSurface * surface = 0) override;

private:
	ScriptInterpreter * interpreter;
	Gpu::Font * displayFont;
	Gpu::Font * shadowFont;
	std::string inputString;
};

} // namespace Ingenuity

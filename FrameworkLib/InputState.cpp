#include "stdafx.h"
#include "InputState.h"

#define _X86_
#include <Windows.h>
#undef _X86_

#include <sstream>

namespace Ingenuity {

void InputState::Update()
{
	mouse.dX = float(mouse.x) - float(mouse.prevX);
	mouse.dY = float(mouse.y) - float(mouse.prevY);

	mousePrev = mouse;
	mouse.prevX = mouse.x;
	mouse.prevY = mouse.y;
	mouse.leftDown = mouse.leftUp =
		mouse.middleDown = mouse.middleUp =
		mouse.rightDown = mouse.rightUp =
		mouse.entered = mouse.exited = false;

	keyboardPrev = keyboard;
	memset(keyboard.upKeys, 0, sizeof(bool[MAX_KEYS]));
	memset(keyboard.downKeys, 0, sizeof(bool[MAX_KEYS]));
	keyboard.text = "";
}

} // namespace Intenuity

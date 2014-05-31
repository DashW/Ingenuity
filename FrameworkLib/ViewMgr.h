// Abstract class to manage window operations across all operating systems, e.g. Win32,WinRT,Gnome
// Assumes that the game engine will only have ONE window through its entire operation
#pragma once

struct ViewMgr_Response
{
	virtual void Respond() = 0;
};

class ViewMgr
{
public:
	ViewMgr();
	~ViewMgr();

	virtual void ProcessEvents() {};

	ViewMgr_Response* onDestroy;
	ViewMgr_Response* onResizing;
	ViewMgr_Response* onMinmize;
	ViewMgr_Response* onMaximize;
	ViewMgr_Response* onUnmaximize;
	ViewMgr_Response* onExitSizeMove;
	ViewMgr_Response* onClose;
	ViewMgr_Response* onActivate;
	ViewMgr_Response* onKeyDown;
	ViewMgr_Response* onMouseLeftDown;
};
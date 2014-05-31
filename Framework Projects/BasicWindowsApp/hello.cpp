#include <Windows.h>


HWND globalWindowHandle = 0;
bool InitWindowsApp(HINSTANCE instanceHandle, int show);
int Run();
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmdLine, int nShowCmd)
{
	if(!InitWindowsApp(instance, nShowCmd))
	{
		MessageBox(0, "Initialisation failed!", "Error!", MB_OK);
		return 1;
	}

	return Run();
}

bool InitWindowsApp(HINSTANCE instanceHandle, int show)
{
	WNDCLASS wc;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instanceHandle;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszMenuName = 0;
	wc.lpszClassName = "Hello";

	if(!RegisterClass(&wc)) 
	{
		MessageBox(0, "Class Registration Failed!", "Error", 0);
		return false;
	}

	globalWindowHandle = CreateWindow(
		"Hello", // Class Name
		"Hello", // Window Name
		WS_OVERLAPPEDWINDOW, // Window Style
		CW_USEDEFAULT, // X pos
		CW_USEDEFAULT, // Y pos
		CW_USEDEFAULT, // Width
		CW_USEDEFAULT, // Height
		0, // Parent Window
		0, // Menu
		instanceHandle, // Instance
		0); // lParam?

	if(!globalWindowHandle) 
	{
		MessageBox(0, "Window Creation Failed", "Error", 0);
		return false;
	}

	ShowWindow(globalWindowHandle, show);
	UpdateWindow(globalWindowHandle);

	return true;
}

int Run()
{
	MSG msg;

	ZeroMemory(&msg, sizeof(MSG));

	while(GetMessage(&msg, 0, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_LBUTTONDOWN:
		MessageBox(0, "Hello World", "Hello", MB_OK);
		return 0;
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE)
		{
			DestroyWindow(globalWindowHandle);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(handle,msg,wParam,lParam);
}
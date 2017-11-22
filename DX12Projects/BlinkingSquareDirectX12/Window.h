#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

class Window
{
public:
	Window(HINSTANCE hInstance, LPCTSTR windowName, LPCTSTR windowTitle, int nShowCmd, int width, int height, bool fullscreen);
	~Window();
	HWND GetHandle();

private:
	HINSTANCE hInstance;
	LPCTSTR windowName;
	LPCTSTR windowTitle;
	int width;
	int height;
	int nShowCmd;
	bool fullscreen;
	HWND hwnd;
	static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool IntializeWindow();
};
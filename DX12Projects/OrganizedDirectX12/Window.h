#pragma once
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
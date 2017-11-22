#pragma once

#include <windows.h>


class TargetWindow
{
public:
	HWND hwnd = NULL;
	// Set the name of the window (NOT THE TITLE)
	LPCTSTR WindowName = "I Want To Become A Winged Hussar";
	// Should be obvious what this is (this is the title of the window)
	LPCTSTR WindowTitle = "THEN THE WINGED HUSSARS ARRIVED";

	// Set width and height of ze window
	int Width = 1000;
	int Height = 800;

	// fullscreen mode
	bool FullScreen = false;
	bool Running = true;

	int WINAPI WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nShowCmd);

	bool IntializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);

	static LRESULT CALLBACK WndProc(HWND hWnd,
		UINT msg,
		WPARAM wParam,
		LPARAM lParam);

};
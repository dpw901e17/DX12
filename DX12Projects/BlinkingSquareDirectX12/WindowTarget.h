#pragma once
#include <Windows.h>

class WindowTarget
{
public:
	HWND hwnd;
	
	virtual void CreateDevice() = 0;
	WindowTarget(HINSTANCE hInstance, LPCTSTR windowName, LPCTSTR windowTitle, int width, int height, bool fullscreen);
	~WindowTarget();
	bool IntializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);

private:
	LPCTSTR windowName;
	LPCTSTR windowTitle;
	int width;
	int height;
	bool fullScreen;
};

class DX12Window : public WindowTarget {
public:
	void CreateDevice() override;
	DX12Window(HINSTANCE hInstance, LPCTSTR windowName, LPCTSTR windowTitle, int width, int height, bool fullscreen);
	~DX12Window();
};


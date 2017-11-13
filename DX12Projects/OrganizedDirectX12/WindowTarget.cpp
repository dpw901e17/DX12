#include "WindowTarget.h"
#include "Window.h"

// Window class
WindowTarget::WindowTarget(HINSTANCE hInstance, LPCTSTR windowName, LPCTSTR windowTitle, int width, int height, bool fullscreen){
	this->windowName = windowName;
	this->windowTitle = windowTitle;
	this->width = width;
	this->height = height;
	this->fullScreen = fullScreen;
}

WindowTarget::~WindowTarget()
{
	// Add cleanup later
}

bool WindowTarget::IntializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen)
{
	return false;
}

// DX12 Class
DX12Window::DX12Window(HINSTANCE hInstance, LPCTSTR windowName, LPCTSTR windowTitle, int width, int height, bool fullscreen)
	: WindowTarget(hInstance, windowName, windowTitle, width, height, fullscreen)
{

}

DX12Window::~DX12Window() {
	//

}

void DX12Window::CreateDevice(){

}



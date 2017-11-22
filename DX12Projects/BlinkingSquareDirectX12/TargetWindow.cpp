#include "TargetWindow.h"

bool TargetWindow::IntializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen)
{
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "ERROR - could not register window class", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	//Attempt to create window handle
	hwnd = CreateWindowEx(NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL, NULL,
		hInstance, NULL);
	//if we failed, report it.
	if (!hwnd)
	{
		MessageBox(NULL, "ERROR - could not create window", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}
	//if we are fullscreened, do it windowed fullscreen.
	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	//if everything went well, show the window.
	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

LRESULT TargetWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
		switch (msg)
		{
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				if (MessageBox(0, "Are you a quitter?", "QUEST: A true quitter", MB_YESNO | MB_ICONQUESTION) == IDYES)
					DestroyWindow(hwnd);
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*
int TargetWindow::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (!IntializeWindow(hInstance, nShowCmd, Width, Height, FullScreen))
	{
		MessageBox(0, "Window was not intialized - YOU FAILED", "ERROR", MB_OK);
		return 0;
	}

	if (!InitD3D())
	{
		MessageBox(0, "intialization of direct3d 12 failed", "You failed", MB_OK);
		Cleanup();
		return 1;
	}

	//PlaySound("wh.wav", NULL, SND_FILENAME | SND_ASYNC);

	mainloop();

	//Cleanup gpu.
	WaitForPreviousFrame();
	CloseHandle(fenceEvent);

	return 0;
}
*/
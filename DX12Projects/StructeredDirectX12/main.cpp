#include <iostream>
#include "DX12Renderer.h"
#include "TargetWindow.h"

using namespace DirectX;

DX12Renderer dx12r;
TargetWindow tw;

void mainloop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//run gamecode
			dx12r.Update();
			dx12r.Render(tw);
		}
	}

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{


	if (!tw.IntializeWindow(hInstance, nShowCmd, tw.Width, tw.Height, tw.FullScreen))
	{
		MessageBox(0, "Window was not intialized - YOU FAILED", "ERROR", MB_OK);
		return 0;
	}
	
	if (!dx12r.InitD3D(tw))
	{
		MessageBox(0, "intialization of direct3d 12 failed", "You failed", MB_OK);
		dx12r.Cleanup(tw);
		return 1;
	}
	
	PlaySound("wh.wav", NULL, SND_FILENAME | SND_ASYNC);

	mainloop();
	
	//Cleanup gpu.
	dx12r.WaitForPreviousFrame(tw);
	CloseHandle(dx12r.fenceEvent);

	return 0;
}
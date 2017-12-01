#pragma once
#include "d3dx12.h"
#include <dxgi1_4.h>
#include  "../../scene-window-system/Window.h"

class SwapChainHandler {
public:
	SwapChainHandler(IDXGIFactory4* dxgiFactory, DXGI_SAMPLE_DESC sampleDesc, int frameBufferCount, Window window, ID3D12CommandQueue* commandQueue);
	~SwapChainHandler();

	
	IDXGISwapChain3* GetSwapChain();

private:
	IDXGISwapChain3* swapChain;

};
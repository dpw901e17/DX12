#include "SwapChainHandler.h"


SwapChainHandler::SwapChainHandler(IDXGIFactory4* dxgiFactory, DXGI_SAMPLE_DESC sampleDesc, int frameBufferCount, Window window, ID3D12CommandQueue* commandQueue) {
	DXGI_MODE_DESC backBufferDesc = {}; 

	backBufferDesc.Width = window.width(); 

		backBufferDesc.Height = window.height(); 

	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; 

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; 

	swapChainDesc.OutputWindow = window.GetHandle(); 

	swapChainDesc.SampleDesc = sampleDesc; 

	swapChainDesc.Windowed = !window.GetFullscreen(); 


	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(
		commandQueue,
		&swapChainDesc,
		&tempSwapChain 

	);
	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
}

SwapChainHandler::~SwapChainHandler() {

}

IDXGISwapChain3* SwapChainHandler::GetSwapChain() {
	return swapChain;
}

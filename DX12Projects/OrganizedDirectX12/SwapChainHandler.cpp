#include "SwapChainHandler.h"


SwapChainHandler::SwapChainHandler(IDXGIFactory4* dxgiFactory, DXGI_SAMPLE_DESC sampleDesc, int frameBufferCount, Window window, ID3D12CommandQueue* commandQueue) {
	// -- Create Swap Chain with tripple buffering -- //
	DXGI_MODE_DESC backBufferDesc = {}; // Describes our display mode
	backBufferDesc.Width = window.width(); // buffer width
		backBufferDesc.Height = window.height(); // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // buffer format. rgba 32 bit.
														// Multisampling. Not really using it, but we need at least one sample from buffer to display
														// Swap chain description
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // pipeline render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // buffer data discarded after present
	swapChainDesc.OutputWindow = window.GetHandle(); // window handle
	swapChainDesc.SampleDesc = sampleDesc; // multi-sampling descriptor
	swapChainDesc.Windowed = !window.GetFullscreen(); // Apperantly more complicated than it looks

	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(
		commandQueue,
		&swapChainDesc,
		&tempSwapChain // Swap chain interface stored here
	);

	// Cast made so that we get a swapchain3. Allowing for calls to GetCurrentBackBufferIndex
	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
}

SwapChainHandler::~SwapChainHandler() {

}

IDXGISwapChain3* SwapChainHandler::GetSwapChain() {
	return swapChain;
}

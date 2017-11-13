#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include "TargetWindow.h"
#include <iostream>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>
#include <Mmsystem.h>
#include <mciapi.h>

// this will only call release if an object exists (prevents exceptions calling release on non existant objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

class DX12Renderer
{
//*********
//DirectX12 variables

	static const int frameBufferCount = 3;
	ID3D12Device* device; //the direct3d device
	IDXGISwapChain3* swapChain; //swapchain to use for rendering
	ID3D12CommandQueue* commandQueue; //
	ID3D12DescriptorHeap* rtvDescriptorHeap;
	ID3D12Resource* renderTargets[frameBufferCount];
	ID3D12CommandAllocator* commandAllocator[frameBufferCount];
	ID3D12GraphicsCommandList* commandList;
	ID3D12Fence* fence[frameBufferCount];

	public: HANDLE fenceEvent;
	UINT64 fenceValue[frameBufferCount];

	int frameIndex;

	int rtvDescriptorSize;

//Index buffering
	ID3D12Resource* indexBuffer;

	D3D12_INDEX_BUFFER_VIEW indexBufferView;

//******* Tutorial 4

	ID3D12PipelineState* pipelineStateObject;
	ID3D12RootSignature* rootSignature;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	ID3D12Resource* vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

//*********
//DirectX12 functions
public:
	bool InitD3D(TargetWindow& tw);
	void Update();
	void UpdatePipeline(TargetWindow& tw);
	void Render(TargetWindow& tw);
	void Cleanup(TargetWindow& tw);
	void WaitForPreviousFrame(TargetWindow& tw);
};
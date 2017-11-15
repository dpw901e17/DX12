#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <iostream>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <string>
#include <Mmsystem.h>
#include <mciapi.h>
#include "WindowTarget.h"

// this will only call release if an object exists (prevents exceptions calling release on non existant objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

//*********
//DirectX12 variables

const int frameBufferCount = 3;
ID3D12Device* device; //the direct3d device
IDXGISwapChain3* swapChain; //swapchain to use for rendering
ID3D12CommandQueue* commandQueue; //
ID3D12DescriptorHeap* rtvDescriptorHeap;
ID3D12Resource* renderTargets[frameBufferCount];
ID3D12CommandAllocator* commandAllocator[frameBufferCount];
ID3D12GraphicsCommandList* commandList;
ID3D12Fence* fence[frameBufferCount];

HANDLE fenceEvent;
UINT64 fenceValue[frameBufferCount];

int frameIndex;

int rtvDescriptorSize;


//******* Tutorial 4

ID3D12PipelineState* pipelineStateObject;
ID3D12RootSignature* rootSignature;
D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;
ID3D12Resource* vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

//********* Tutorial 5 indexes
ID3D12Resource* indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

//********* Tutorial 6 depth test
ID3D12Resource* depthStencilBuffer; 
ID3D12DescriptorHeap* dsDescriptorHeap;

//*********
//DirectX12 functions

bool InitD3D();
void Update();
void UpdatePipeline();
void Render();
void Cleanup();
void WaitForPreviousFrame();


//*********
//Window Variables

// We need a window handler
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

//*********

// Main loop of the application
void mainloop();

struct Vertex {
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, a) {}
	Vertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT4 color) : pos(position), color(color) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};

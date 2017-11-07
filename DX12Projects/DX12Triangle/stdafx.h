// Collects headers into one. For funz.
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

//data collection
#include <chrono>
#include "wmiaccess.h"
#include <fstream>

// Only call release if object exists.
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

//exeArgs
bool argCsv = false;
bool argPerfmon = false;
bool argPipelineStat = false;
int argTime = 0;

// Attributes
HWND hwnd = NULL; // Window Handle
LPCTSTR WindowName = "Brandborg";
LPCTSTR WindowTitle = "Brandborg";

int Width = 800;
int Height = 600;
bool FullScreen = false;
bool Running = true;

// Functions
bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen);

void mainloop();

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// direct3d
const int frameBufferCount = 3; // triple buffering
ID3D12Device* device;
IDXGISwapChain3* swapChain;
ID3D12CommandQueue* commandQueue; // To hold commands
ID3D12DescriptorHeap* rtvDescriptorHeap; // To hold resources
ID3D12Resource* renderTargets[frameBufferCount];
ID3D12CommandAllocator* commandAllocator[frameBufferCount];
ID3D12GraphicsCommandList* commandList;
ID3D12Fence* fence[frameBufferCount];

HANDLE fenceEvent; // Handle to event at GPU unlock
UINT64 fenceValue[frameBufferCount];
int frameIndex; // Current frame to render to
int rtvDescriptorSize; // Size of rtv descriptor 

// Drawing! Globals
ID3D12PipelineState* pipelineStateObject;
ID3D12RootSignature* rootSignature;
D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;
ID3D12Resource* vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

//Pipeline statistics query:
ID3D12QueryHeap* queryHeap;
ID3D12Resource* queryResult;
ID3D12PipelineState* queryState;

struct Vertex {
	DirectX::XMFLOAT3 pos;
};

// function declarations
bool InitD3D();
void Update();
void UpdatePipeline();
void Render(long long timestamp);
void Cleanup();
void WaitForPreviousFrame();

//timing stuff:
void Arrange_Test_Data(const std::string* dataArr, WMIDataItem* item);
void SaveToFile(const std::string& file, const std::string& data);
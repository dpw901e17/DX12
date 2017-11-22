#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#define STB_IMAGE_IMPLEMENTATION

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
#include "stb_image.h"
#include "../../scene-window-system/Scene.h"

// this will only call release if an object exists (prevents exceptions calling release on non existant objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

struct ConstantBuffer {
	DirectX::XMFLOAT4 colorMultiplier;
};

struct ConstantBufferPerObject {
	DirectX::XMFLOAT4X4 wvpMat;
};

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

//********* Tutorial 9 Cube

// Allignment value
int ConstantBufferPerObjectAllignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255; //  ~255 = -256 = 1000000000

ConstantBufferPerObject cbPerObject; // constant buffer data sent to the GPU

ID3D12Resource* constantBufferUploadHeaps[frameBufferCount]; // gpu memory where the buffer is placed

UINT8* cbvGPUAddress[frameBufferCount]; // pointer to constant buffer resource heaps

										// Matrixes
										// Remember not to pass around the matrixes. But store them in vectors first. 
DirectX::XMFLOAT4X4 cameraProjMat;
DirectX::XMFLOAT4X4 cameraViewMat;

DirectX::XMFLOAT4 cameraPosition;
DirectX::XMFLOAT4 cameraTarget;
DirectX::XMFLOAT4 cameraUp;

DirectX::XMFLOAT4X4 cube1WorldMat;
DirectX::XMFLOAT4X4 cube1RotMat;
DirectX::XMFLOAT4 cube1Position;

DirectX::XMFLOAT4X4 cube2WorldMat;
DirectX::XMFLOAT4X4 cube2RotMat;
DirectX::XMFLOAT4 cube2PositionOffset;

int numCubeIndices;

//********Tutorial 10 texture

ID3D12Resource* textureBuffer;

int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, char* filename, int &bytesPerRow);

/*
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGuid);
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
*/

ID3D12DescriptorHeap* mainDescriptorHeap;
ID3D12Resource* textureBufferUploadHeap;

//Scene Objects
Scene* basicBoxScene;

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
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
};

/*
struct Vertex {
	Vertex(float x, float y, float z, float r, float g, float b, float a) : pos(x, y, z), color(r, g, b, a) {}
	Vertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT4 color) : pos(position), color(color) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};
*/


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
#include <vector>

#include "SafeRelease.h"
#include "Device.h"
#include "ShaderHandler.h"
#include "SwapChainHandler.h"
#include "ResourceHandler.h"
#include "CubeContainer.h"
#include "ResourceFactory.h"
#include "PipelineStateHandler.h"
#include <array>


//*********
//DirectX12 variables

const int frameBufferCount = 3;
//ID3D12Device* device; //the direct3d device
//IDXGISwapChain3* swapChain; //swapchain to use for rendering
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
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

//********* Tutorial 5 indexes
ID3D12Resource* indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

//********* Tutorial 6 depth test
ID3D12Resource* depthStencilBuffer;
ID3D12DescriptorHeap* dsDescriptorHeap;

//********* Tutorial 9 Cube


// Matrixes
// Remember not to pass around the matrixes. But store them in vectors first. 

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
uint64_t numOfFrames = 0;


//*********
//DirectX12 functions

void InitD3D(Window window);
void Update();
void UpdatePipeline();
void Render(SwapChainHandler swapChainHandler);
void Cleanup(SwapChainHandler swapChainHandler);
void WaitForPreviousFrame(SwapChainHandler swapChainHandler);


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

Vertex vList[] = {
	// front face
	{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
	{ 0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f,  0.5f, -0.5f, 1.0f, 0.0f },

	// right side face
	{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
	{ 0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
	{ 0.5f,  0.5f, -0.5f, 0.0f, 0.0f },

	// left side face
	{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ -0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
	{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f },

	// back face
	{ 0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
	{ -0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
	{ 0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f, 1.0f, 0.0f },

	// top face
	{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
	{ 0.5f,  0.5f, -0.5f, 1.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },

	// bottom face
	{ 0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ -0.5f, -0.5f,  0.5f, 1.0f, 0.0f },
};

// Creating the index buffer
DWORD iList[] = {
	// ffront face
	0, 1, 2, // first triangle
	0, 3, 1, // second triangle

	// left face
	4, 5, 6, // first triangle
	4, 7, 5, // second triangle

	// right face
	8, 9, 10, // first triangle
	8, 11, 9, // second triangle

	// back face
	12, 13, 14, // first triangle
	12, 15, 13, // second triangle

		// top face
		16, 17, 18, // first triangle
		16, 19, 17, // second triangle

					// bottom face
					20, 21, 22, // first triangle
					20, 23, 21, // second triangle
};

//******************TEMPORARY GLOBALS****************************
Device* globalDevice;
SwapChainHandler* globalSwapchain;
ResourceHandler* globalVertexDefaultHeap;
ResourceHandler* globalVertexUploadHeap;
CubeContainer* globalCubeContainer;
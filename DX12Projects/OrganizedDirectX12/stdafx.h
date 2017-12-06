#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             

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
#include <array>

#include "SafeRelease.h"
#include "Device.h"
#include "ShaderHandler.h"
#include "SwapChainHandler.h"
#include "ResourceHandler.h"
#include "CubeContainer.h"
#include "ResourceFactory.h"
#include "PipelineStateHandler.h"
#include "CommandListHandler.h"




const int frameBufferCount = 3;
ID3D12CommandQueue* commandQueue; 

ID3D12DescriptorHeap* rtvDescriptorHeap;
ID3D12Resource* renderTargets[frameBufferCount];
ID3D12CommandAllocator* commandAllocator[frameBufferCount];
ID3D12GraphicsCommandList* commandList;
ID3D12Fence* fence[frameBufferCount];

HANDLE fenceEvent;
UINT64 fenceValue[frameBufferCount];

int frameIndex;

int rtvDescriptorSize;



ID3D12PipelineState* pipelineStateObject;
ID3D12RootSignature* rootSignature;
D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

ID3D12Resource* indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

ID3D12Resource* depthStencilBuffer;
ID3D12DescriptorHeap* dsDescriptorHeap;




int numCubeIndices;


ID3D12Resource* textureBuffer;

int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, char* filename, int &bytesPerRow);


ID3D12DescriptorHeap* mainDescriptorHeap;
ID3D12Resource* textureBufferUploadHeap;

Scene* basicBoxScene;
uint64_t numOfFrames = 0;



void InitD3D(Window window);
void Update();
void UpdatePipeline();
void Render(SwapChainHandler swapChainHandler);
void Cleanup(SwapChainHandler swapChainHandler);
void WaitForPreviousFrame(SwapChainHandler swapChainHandler);



HWND hwnd = NULL;

LPCTSTR WindowName = "I Want To Become A Winged Hussar";
LPCTSTR WindowTitle = "THEN THE WINGED HUSSARS ARRIVED";


int Width = 1000;
int Height = 800;

bool FullScreen = false;

bool Running = true;


void mainloop();

struct Vertex {
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
};

Vertex vList[] = {
	{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
	{ 0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f,  0.5f, -0.5f, 1.0f, 0.0f },
	{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
	{ 0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
	{ 0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
	{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ -0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
	{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f },
	{ 0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
	{ -0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
	{ 0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
	{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
	{ 0.5f,  0.5f, -0.5f, 1.0f, 1.0f },
	{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
	{ 0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ -0.5f, -0.5f,  0.5f, 1.0f, 0.0f },
};

DWORD iList[] = {
	0, 1, 2, 

	0, 3, 1, 
	4, 5, 6, 

	4, 7, 5, 
	8, 9, 10, 

	8, 11, 9, 
	12, 13, 14, 

	12, 15, 13, 
		16, 17, 18, 

		16, 19, 17, 
					20, 21, 22, 

					20, 23, 21, 

};

Device* globalDevice;
SwapChainHandler* globalSwapchain;
ResourceHandler* globalVertexDefaultHeap;
ResourceHandler* globalVertexUploadHeap;
CubeContainer* globalCubeContainer;
PipelineStateHandler* globalPipeline;
PipelineStateHandler* globalPipeline2;
CommandListHandler* globalCommandListHandler;
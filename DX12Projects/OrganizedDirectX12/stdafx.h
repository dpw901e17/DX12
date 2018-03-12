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
#include "../../scene-window-system/TestConfiguration.h"
#include "../../scene-window-system/WmiAccess.h"
#include "../../scene-window-system/ThreadPool.hpp"
#include <vector>
#include <array>
#include <chrono>
#include <sstream>
#include <fstream>

#include "SafeRelease.h"
#include "Device.h"
#include "ShaderHandler.h"
#include "SwapChainHandler.h"
#include "ResourceHandler.h"
#include "CubeContainer.h"
#include "ResourceFactory.h"
#include "PipelineStateHandler.h"
#include "CommandListHandler.h"




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


ID3D12DescriptorHeap* mainDescriptorHeap;
ID3D12Resource* textureBufferUploadHeap;

//Scene Objects
Scene* basicBoxScene;


//*********
//DirectX12 functions

void InitD3D(Window window);
void Update();
void UpdatePipeline(TestConfiguration testConfig);
void Render(SwapChainHandler swapChainHandler, TestConfiguration testConfig);
void Cleanup(SwapChainHandler swapChainHandler);
void WaitForPreviousFrame(SwapChainHandler& swapChainHandler);


//*********
//Window Variables

// We need a window handler
HWND hwnd = NULL;

// Set the name of the window (NOT THE TITLE)
LPCTSTR WindowName = "I Want To Become A Winged Hussar";
// Should be obvious what this is (this is the title of the window)
LPCTSTR WindowTitle = "THEN THE WINGED HUSSARS ARRIVED";


// Set width and height of ze window
int Width = 800;
int Height = 600;

// fullscreen mode
bool FullScreen = false;

bool Running = true;

//*********

// Main loop of the application
void mainloop(DataCollection<WMIDataItem>& wmiDataCollection, DataCollection<PipelineStatisticsDataItem>& pipelineStatisticsDataCollection, TestConfiguration& testConfig, Window* window);

struct Vertex {
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 texCoord;
};

//vertex data: cube
#define TEST_USE_CUBE

#include "VertexCube.h"
#include "VertexSkull.h"

#include "IndexCube.h"
#include "IndexSkull.h"

//******************TEMPORARY GLOBALS****************************
Device* globalDevice;
SwapChainHandler* globalSwapchain;
ResourceHandler* globalVertexDefaultHeap;
ResourceHandler* globalVertexUploadHeap;
CubeContainer* globalCubeContainer;
PipelineStateHandler* globalPipeline;
PipelineStateHandler* globalPipeline2;
CommandListHandler* globalCommandListHandler;
CommandListHandler* globalCommandListHandler2;
CommandListHandler* globalStartCommandListHandler;
CommandListHandler* globalEndCommandListHandler;

std::vector<CommandListHandler*> drawCommandLists;

//pipeline statistics:
ID3D12QueryHeap* globalQueryHeap;
ID3D12Resource* globalQueryResult;
D3D12_QUERY_DATA_PIPELINE_STATISTICS* globalQueryBuffer;

struct DrawCubesInfo
{
	int frameIndex;
	CommandListHandler* commandListHandler;
	PipelineStateHandler* pipelineStateHandler;
	size_t cubeCount;
	size_t cubeStride;
	CubeContainer* globalCubeContainer;
	ID3D12Resource** renderTargets;	//TODO: arbitrary number of backbuffers?
	ID3D12DescriptorHeap* rtvDescriptorHeap;
	int rtvDescriptorSize;
	ID3D12DescriptorHeap* dsDescriptorHeap;
	ID3D12RootSignature* rootSignature;
	ID3D12DescriptorHeap* mainDescriptorHeap;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	D3D12_VERTEX_BUFFER_VIEW* vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW* indexBufferView;
	int numCubeIndices;
	size_t drawStartIndex;
	int queryIndex; //<-- used for pipeline statistics
};


void DrawCubes(DrawCubesInfo& info);

ThreadPool* globalThreadPool;

template<typename T>
auto force_string(T arg) {
	std::stringstream ss;
	ss << arg;
	return ss.str();
}

void SaveToFile(const std::string& file, const std::string& data) 
{
	std::ofstream fs;
	fs.open(file, std::ofstream::app);
	fs << data;
	fs.close();
}

void Arrange_OHM_Data(const std::string* dataArr, WMIDataItem* item);
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
#include "../../scene-window-system/TestConfiguration.h"
#include "../../scene-window-system/WmiAccess.h"
#include "../../scene-window-system/ThreadPool.h"
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


void InitD3D(Window window);
void Update();
void UpdatePipeline(TestConfiguration testConfig);
void Render(SwapChainHandler swapChainHandler, TestConfiguration testConfig);
void Cleanup(SwapChainHandler swapChainHandler);
void WaitForPreviousFrame(SwapChainHandler& swapChainHandler);


HWND hwnd = NULL;

LPCTSTR WindowName = "I Want To Become A Winged Hussar";
LPCTSTR WindowTitle = "THEN THE WINGED HUSSARS ARRIVED";


int Width = 800;
int Height = 600;

bool FullScreen = false;

bool Running = true;

void mainloop(DataCollection<WMIDataItem>& wmiDataCollection, DataCollection<PipelineStatisticsDataItem>& pipelineStatisticsDataCollection, TestConfiguration& testConfig, Window* window);

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
	{ -0.5f, -0.5f,  0.5f, 1.0f, 0.0f }
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
CommandListHandler* globalCommandListHandler2;
CommandListHandler* globalStartCommandListHandler;
CommandListHandler* globalEndCommandListHandler;

std::vector<CommandListHandler*> drawCommandLists;

ID3D12QueryHeap* globalQueryHeap;
ID3D12Resource* globalQueryResult;
D3D12_QUERY_DATA_PIPELINE_STATISTICS* globalQueryBuffer;

struct DrawCubesInfo
{
	int frameIndex;
	CommandListHandler* commandListHandler;
	PipelineStateHandler* pipelineStateHandler;
	size_t cubeCount;
	CubeContainer* globalCubeContainer;
	ID3D12Resource** renderTargets;	    
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
	int queryIndex;     
};


void DrawCubes(DrawCubesInfo& info);

ThreadPool<DrawCubesInfo>* globalThreadPool;

typedef void (*RenderJob) (DrawCubesInfo&);

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
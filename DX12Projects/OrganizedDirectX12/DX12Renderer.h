#pragma once
#include "stdafx.h"

class DX12Renderer
{
public:
	DX12Renderer();
	~DX12Renderer();


private:
	const int frameBufferCount = 3;
	ID3D12Device* device; 
	IDXGISwapChain3* swapChain; 
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
	int numCubeIndices;

	//********* Tutorial 6 depth test
	ID3D12Resource* depthStencilBuffer;
	ID3D12DescriptorHeap* dsDescriptorHeap;

	//********* Tutorial 9 Cube

	// Allignment value
	int ConstantBufferPerObjectAllignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

	// constant buffer data sent to the GPU
	std::vector<ConstantBufferPerObject> constantBuffers[frameBufferCount];

	// gpu memory where the buffer is placed
	ID3D12Resource* constantBufferUploadHeaps[frameBufferCount]; 

	// pointer to constant buffer resource heaps
	UINT8* cbvGPUAddress[frameBufferCount]; 

	// Matrixes
	// Remember not to pass around the matrixes. But store them in vectors first. 
	DirectX::XMFLOAT4X4 cameraProjMat;
	DirectX::XMFLOAT4X4 cameraViewMat;

	DirectX::XMFLOAT4 cameraPosition;
	DirectX::XMFLOAT4 cameraTarget;
	DirectX::XMFLOAT4 cameraUp;

	// Contains position, rotation and world matrices for each cube
	std::vector<CubeMatrices> cubeMatrices;

	//********Tutorial 10 texture
	ID3D12Resource* textureBuffer;
	ID3D12DescriptorHeap* mainDescriptorHeap;
	ID3D12Resource* textureBufferUploadHeap;

	int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, char* filename, int &bytesPerRow);


	//********Scene Objects
	Scene* basicBoxScene;

	//*********
	//DirectX12 functions
	void InitD3D();
	void Update();
	void UpdatePipeline();
	void Render();
	void Cleanup();
	void WaitForPreviousFrame();


	//*********Windows
	HWND hwnd = NULL; // handle to window

	//*********Refactoring
};

DX12Renderer::DX12Renderer()
{
}

DX12Renderer::~DX12Renderer()
{
}
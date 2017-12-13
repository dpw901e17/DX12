#pragma once
#include "CubeContainer.h"
#include <string>

class CommandListHandler {
public:
	CommandListHandler(const Device& device, int frameBufferCount);
	~CommandListHandler();
	
	void SetState(ID3D12Resource * renderTargets[], ID3D12DescriptorHeap & rtvDescriptorHeap, int rtvDescriptorSize, ID3D12DescriptorHeap & dsDescriptorHeap, ID3D12RootSignature & rootSignature, ID3D12DescriptorHeap & mainDescriptorHeap, D3D12_VIEWPORT & viewport, D3D12_RECT & scissorRect, D3D12_VERTEX_BUFFER_VIEW & vertexBufferView, D3D12_INDEX_BUFFER_VIEW & indexBufferView);
	void RecordDrawCalls(const CubeContainer& cubeContainer, int numCubeIndices);
	void RecordOpen(ID3D12Resource* renderTargets[]);
	void RecordClosing(ID3D12Resource * renderTargets[]);
	void RecordClearScreenBuffers(ID3D12DescriptorHeap & rtvDescriptorHeap, int rtvDescriptorSize, ID3D12DescriptorHeap & dsDescriptorHeap);
	void Open(int frameBufferIndex, ID3D12PipelineState& pipeline);
	void Close();
	ID3D12GraphicsCommandList* GetCommandList() const;

	//TODO: remove this?
	std::string& GetGPUCommandDebugString() { return gpuCommandDebug; };

private:
	int m_frameBufferIndex;
	std::vector<ID3D12CommandAllocator*> m_commandAllocators;
	ID3D12GraphicsCommandList* m_commandList;
	void CreateCommandAllocators(const Device& device, int frameBufferCount);
	
	//TODO: remove this?
	std::string gpuCommandDebug;
};
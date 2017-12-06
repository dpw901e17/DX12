#pragma once
#include "CubeContainer.h"

class CommandListHandler {
public:
	CommandListHandler(const Device& device, int frameBufferCount);
	~CommandListHandler();
	
	void RecordSetup(ID3D12Resource * renderTargets[], ID3D12DescriptorHeap & rtvDescriptorHeap, int rtvDescriptorSize, ID3D12DescriptorHeap & dsDescriptorHeap, ID3D12RootSignature & rootSignature, ID3D12DescriptorHeap & mainDescriptorHeap, D3D12_VIEWPORT & viewport, D3D12_RECT & scissorRect, D3D12_VERTEX_BUFFER_VIEW & vertexBufferView, D3D12_INDEX_BUFFER_VIEW & indexBufferView, bool clearScreen);
	void RecordDrawCalls(const CubeContainer& cubeContainer, int numCubeIndices);
	void RecordClosing(ID3D12Resource * renderTargets[]);
	void Open(int frameBufferIndex, ID3D12PipelineState& pipeline);
	void Close();
	ID3D12GraphicsCommandList* GetCommandList() const;

private:
	int m_frameBufferIndex;
	std::vector<ID3D12CommandAllocator*> m_commandAllocators;
	ID3D12GraphicsCommandList* m_commandList;
	void CreateCommandAllocators(const Device& device, int frameBufferCount);
};
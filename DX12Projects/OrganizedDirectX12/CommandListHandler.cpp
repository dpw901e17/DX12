#include "CommandListHandler.h"
#include <sstream>

CommandListHandler::CommandListHandler(const Device & device, int frameBufferCount)
{
	CreateCommandAllocators(device, frameBufferCount);
	HRESULT hr;
	hr = device.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, *m_commandAllocators.data(), NULL, IID_PPV_ARGS(&m_commandList));
	if (FAILED(hr))
	{
		throw std::runtime_error("Create command list failed");
	}

	Close();
	m_frameBufferIndex = 0;
}

CommandListHandler::~CommandListHandler()
{
	SAFE_RELEASE(m_commandList);

	for (auto i = 0; i < m_commandAllocators.size(); ++i) {
		SAFE_RELEASE(m_commandAllocators[i]);
	}
}

void CommandListHandler::SetState(ID3D12Resource * renderTargets[], ID3D12DescriptorHeap & rtvDescriptorHeap, int rtvDescriptorSize, ID3D12DescriptorHeap & dsDescriptorHeap, ID3D12RootSignature & rootSignature, ID3D12DescriptorHeap & mainDescriptorHeap, D3D12_VIEWPORT & viewport, D3D12_RECT & scissorRect, D3D12_VERTEX_BUFFER_VIEW & vertexBufferView, D3D12_INDEX_BUFFER_VIEW & indexBufferView)
{
	std::stringstream gpuComDebugStream;
	gpuComDebugStream << gpuCommandDebug;
	gpuComDebugStream << "/******************** CommandListHandler [" << m_commandList << "] :: SetState() ********************/ \r\n";
	HRESULT hr;

	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap.GetCPUDescriptorHandleForHeapStart(), m_frameBufferIndex, rtvDescriptorSize);

	// get handle to depth/stencil buffer
	auto dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsDescriptorHeap.GetCPUDescriptorHandleForHeapStart());

	// Sets destination of output merger.
	// Also sets the depth/stencil buffer for OM use. 
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	gpuComDebugStream << "OMSetRenderTargets(1, &rtvHandle [" << &rtvHandle << "], FALSE, &dsvHandle [" << &dsvHandle << "]) \r\n";

	// set root signature
	m_commandList->SetGraphicsRootSignature(&rootSignature);
	gpuComDebugStream << "SetGraphicsRootSignature(&rootSignature [" << &rootSignature << "]) \r\n";

	//set descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { &mainDescriptorHeap };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	gpuComDebugStream << "SetDescriptorHeaps(_countof(descriptorHeaps) [" << _countof(descriptorHeaps) << "], descriptorHeaps [" << descriptorHeaps << "]) \r\n";

	//set descriptor table index 1 of the root signature. (corresponding to parameter order defined in the signature)
	m_commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap.GetGPUDescriptorHandleForHeapStart());
	gpuComDebugStream << "SetGraphicsRootDescriptorTable(1, mainDescriptorHeap.GetGPUDescriptorHandleForHeapStart() [" << &mainDescriptorHeap.GetGPUDescriptorHandleForHeapStart() << "]) \r\n";

	// draw triangle
	m_commandList->RSSetViewports(1, &viewport);
	gpuComDebugStream << "RSSetViewports(1, &viewport [" << &viewport << "]) \r\n";
	
	m_commandList->RSSetScissorRects(1, &scissorRect);
	gpuComDebugStream << "RSSetScissorRects(1, &scissorRect [" << &scissorRect << "]) \r\n";

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gpuComDebugStream << "IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) \r\n";

	m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	gpuComDebugStream << "IASetVertexBuffers(0, 1, &vertexBufferView [" << &vertexBufferView << "])\r\n";
	
	m_commandList->IASetIndexBuffer(&indexBufferView);
	gpuComDebugStream << "IASetIndexBuffer(&indexBufferView [" << &indexBufferView << "]) \r\n";
	
	gpuComDebugStream << "\r\n";
	gpuCommandDebug = gpuComDebugStream.str();
}



void CommandListHandler::RecordDrawCalls(const CubeContainer& cubeContainer, int numCubeIndices)
{
	std::stringstream gpuComDebugStream;
	gpuComDebugStream << gpuCommandDebug;
	gpuComDebugStream << "/******************** CommandListHandler [" << m_commandList << "] :: RecordDrawCalls() ********************/ \r\n";
	
	for (auto i = 0; i < cubeContainer.GetCubes().size(); ++i) {
		m_commandList->SetGraphicsRootConstantBufferView(0, cubeContainer.GetVirtualAddress(i, m_frameBufferIndex));
		gpuComDebugStream << "SetGraphicsRootConstantBufferView(0, cubeContainer.GetVirtualAddress(i [" << i << "], m_frameBufferIndex [" << m_frameBufferIndex << "]) [" << cubeContainer.GetVirtualAddress(i, m_frameBufferIndex) << "]) \r\n";
		m_commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);
		gpuComDebugStream << "DrawIndexedInstanced(numCubeIndices [" << numCubeIndices << "], 1, 0, 0, 0) \r\n";
	}

	gpuComDebugStream << "\r\n";
	gpuCommandDebug = gpuComDebugStream.str();
}

void CommandListHandler::RecordOpen(ID3D12Resource * renderTargets[])
{
	std::stringstream gpuComDebugStream;
	gpuComDebugStream << gpuCommandDebug;
	gpuComDebugStream << "/******************** CommandListHandler [" << m_commandList << "] :: RecordOpen() ********************/ \r\n";

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[m_frameBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	gpuComDebugStream << "ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[m_frameBufferIndex [" << m_frameBufferIndex << "]] [" << renderTargets[m_frameBufferIndex] << "], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)) \r\n";

	gpuComDebugStream << "\r\n";
	gpuCommandDebug = gpuComDebugStream.str();
}

void CommandListHandler::RecordClosing(ID3D12Resource * renderTargets[])
{
	std::stringstream gpuComDebugStream;
	gpuComDebugStream << gpuCommandDebug;
	gpuComDebugStream << "/******************** CommandListHandler [" << m_commandList << "] :: RecordClosing() ********************/ \r\n";

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[m_frameBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	gpuComDebugStream << "ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[m_frameBufferIndex [" << m_frameBufferIndex << "]] [" << renderTargets[m_frameBufferIndex] << "], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)) \r\n";

	gpuComDebugStream << "\r\n";
	gpuCommandDebug = gpuComDebugStream.str();
}

void CommandListHandler::RecordClearScreenBuffers(ID3D12DescriptorHeap & rtvDescriptorHeap, int rtvDescriptorSize, ID3D12DescriptorHeap & dsDescriptorHeap)
{
	std::stringstream gpuComDebugStream;
	gpuComDebugStream << gpuCommandDebug;
	gpuComDebugStream << "/******************** CommandListHandler [" << m_commandList << "] :: RecordClearScreenBuffers() ********************/ \r\n";

	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap.GetCPUDescriptorHandleForHeapStart(), m_frameBufferIndex, rtvDescriptorSize);

	// Clears screen
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	gpuComDebugStream << "ClearRenderTargetView(rtvHandle [" << &rtvHandle << "], clearColor, 0, nullptr)\r\n";

	// clear the depth/stencil buffer
	m_commandList->ClearDepthStencilView(dsDescriptorHeap.GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	gpuComDebugStream << "ClearDepthStencilView(dsDescriptorHeap.GetCPUDescriptorHandleForHeapStart() ["<< &dsDescriptorHeap.GetCPUDescriptorHandleForHeapStart() << "], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr) \r\n";

	gpuComDebugStream << "\r\n";
	gpuCommandDebug = gpuComDebugStream.str();
}

// Resets the commandlist and the commandallocator for this frame
// Now it is OPEN for business
void CommandListHandler::Open(int frameBufferIndex, ID3D12PipelineState& pipeline)
{
	std::stringstream gpuComDebugStream;
	//gpuComDebugStream << gpuCommandDebug;	//<-- do NOT put old message in stream! (Open() indicates a new round of commands!) 
	gpuComDebugStream << "/******************** CommandListHandler [" << m_commandList << "] :: Open() ********************/ \r\n";


	HRESULT hr;
	m_frameBufferIndex = frameBufferIndex;


	hr = m_commandAllocators[m_frameBufferIndex]->Reset();
	gpuComDebugStream << "m_commandAllocators[m_frameBufferIndex [" << m_frameBufferIndex << "]]->Reset()\r\n";

	if (FAILED(hr))
	{
		throw std::runtime_error("Reset Command Allocator failed");
	}


	// Reset of commands at the GPU and setting of the PSO
	auto tmp = m_commandAllocators[m_frameBufferIndex];
	hr = m_commandList->Reset(tmp, &pipeline);
	gpuComDebugStream << "Reset(m_commandAllocators[m_frameBufferIndex [" << m_frameBufferIndex << "]] [" << m_commandAllocators[m_frameBufferIndex] << "], &pipeline [" << &pipeline << "])\r\n";
	if (FAILED(hr))
	{
		throw std::runtime_error("Reset Command List failed");
	}

	gpuComDebugStream << "\r\n";
	gpuCommandDebug = gpuComDebugStream.str();
}

void CommandListHandler::Close() {
	std::stringstream gpuComDebugStream;
	gpuComDebugStream << gpuCommandDebug;
	gpuComDebugStream << "/******************** CommandListHandler [" << m_commandList << "] :: Close() ********************/ \r\n";

	
	HRESULT hr;
	hr = m_commandList->Close();
	gpuComDebugStream << "Close()\r\n";

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to close command list");
	}

	gpuComDebugStream << "\r\n";
	gpuCommandDebug = gpuComDebugStream.str();
}


void CommandListHandler::CreateCommandAllocators(const Device & device, int frameBufferCount)
{
	HRESULT hr;
	// -- Create Command Allocator -- //
	// One allocator per backbuffer, so that we may free allocators of lists not being executed on GPU.
	// The command list associated will be direct. Not bundled.

	m_commandAllocators.resize(frameBufferCount);
	for (int i = 0; i < frameBufferCount; i++) {
		hr = device.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
		if (FAILED(hr)) {
			throw std::runtime_error("Create command allocator failed.");
		}
	}
}

ID3D12GraphicsCommandList* CommandListHandler::GetCommandList() const {
	return m_commandList;
}
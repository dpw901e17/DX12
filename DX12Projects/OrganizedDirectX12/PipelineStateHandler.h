#pragma once
#include<d3d12.h>
#include "Device.h"
#include "ShaderHandler.h"
#include "SafeRelease.h"

class PipelineStateHandler {
public:
	PipelineStateHandler(const Device& device, const ShaderHandler& shaderHandler, D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc,  DXGI_SAMPLE_DESC& sampleDesc, ID3D12RootSignature& rootSignature);
	~PipelineStateHandler();
	ID3D12PipelineState* GetPipelineStateObject() const;

private:
	ID3D12PipelineState* pipelineStateObject;

};
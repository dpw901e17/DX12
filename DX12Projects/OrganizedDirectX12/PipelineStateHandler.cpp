#include "PipelineStateHandler.h"

PipelineStateHandler::PipelineStateHandler(const Device& device, const ShaderHandler& shaderHandler, D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, DXGI_SAMPLE_DESC& sampleDesc, ID3D12RootSignature& rootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc; 

	psoDesc.pRootSignature = &rootSignature; 

	psoDesc.VS = shaderHandler.GetVertexShaderByteCode(); 

	psoDesc.PS = shaderHandler.GetPixelShaderByteCode(); 

	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; 

	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; 

	psoDesc.SampleDesc = sampleDesc; 

	psoDesc.SampleMask = 0xffffffff;   

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); 

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); 

	psoDesc.NumRenderTargets = 1; 

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); 

	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; 


	HRESULT hr;
	hr = device.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create pipeline");
	}
}

PipelineStateHandler::~PipelineStateHandler()
{
	SAFE_RELEASE(pipelineStateObject);
}

ID3D12PipelineState* PipelineStateHandler::GetPipelineStateObject() const
{
	return pipelineStateObject;
}

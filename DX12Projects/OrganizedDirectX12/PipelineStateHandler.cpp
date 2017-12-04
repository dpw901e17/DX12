#include "PipelineStateHandler.h"

PipelineStateHandler::PipelineStateHandler(const Device& device, const ShaderHandler& shaderHandler, D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, DXGI_SAMPLE_DESC& sampleDesc, ID3D12RootSignature& rootSignature)
{
	

	//Pipeline object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc; // Layout of the vertex buffer
	psoDesc.pRootSignature = &rootSignature; // Pointer to shader accessible data
	psoDesc.VS = shaderHandler.GetVertexShaderByteCode(); // Vertex shader
	psoDesc.PS = shaderHandler.GetPixelShaderByteCode(); // Pixel shader 
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // Drawing triangles
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Format of render target
	psoDesc.SampleDesc = sampleDesc; // Type of multi-sampling to use after render eg. super sampling
	psoDesc.SampleMask = 0xffffffff;   // Mask used in multi-sampling
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // default rasterizer used
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // default blend stage used at the end of pipeline
	psoDesc.NumRenderTargets = 1; // only drawing to one target per pipeline
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // use default depth/stencil buffer
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; // format of depth/stencil buffer

	HRESULT hr;
	hr = device.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create pipeline");
	}
}

PipelineStateHandler::~PipelineStateHandler()
{
}

ID3D12PipelineState* PipelineStateHandler::GetPipelineStateObject() const
{
	return pipelineStateObject;
}

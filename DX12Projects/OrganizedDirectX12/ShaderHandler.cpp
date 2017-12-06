#include "ShaderHandler.h"

ShaderHandler::ShaderHandler(LPCWSTR vertexFileName, LPCWSTR pixelFileName)
{
	vertexShaderBytecode = CompileShaderToBytecode(vertexFileName, "vs_5_0");
	pixelShaderBytecode = CompileShaderToBytecode(pixelFileName, "ps_5_0");
}

ShaderHandler::~ShaderHandler()
{
	
}

D3D12_SHADER_BYTECODE ShaderHandler::CompileShaderToBytecode(LPCWSTR fileName, LPCSTR pTarget)
{
	HRESULT hr;
	ID3DBlob* shader;
	ID3DBlob* errorBuff;
	hr = D3DCompileFromFile(fileName,
		nullptr, nullptr,
		"main",
		pTarget,
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);

	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
	}

	D3D12_SHADER_BYTECODE shaderBytecode = {};
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

	return shaderBytecode;
}

D3D12_SHADER_BYTECODE ShaderHandler::GetPixelShaderByteCode() const
{
	return pixelShaderBytecode;
}

D3D12_SHADER_BYTECODE ShaderHandler::GetVertexShaderByteCode() const
{
	return vertexShaderBytecode;
}

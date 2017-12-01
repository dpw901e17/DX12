#pragma once
#include "d3dx12.h"
#include <D3Dcompiler.h>

class ShaderHandler {

public:
	ShaderHandler(LPCWSTR vertexFileName, LPCWSTR pixelFileName);
	~ShaderHandler();

	D3D12_SHADER_BYTECODE GetPixelShaderByteCode() const; 
	D3D12_SHADER_BYTECODE GetVertexShaderByteCode() const;

private:
	D3D12_SHADER_BYTECODE CompileShaderToBytecode(LPCWSTR fileName, LPCSTR pTarget);
	
	D3D12_SHADER_BYTECODE pixelShaderBytecode;
	D3D12_SHADER_BYTECODE vertexShaderBytecode;
};
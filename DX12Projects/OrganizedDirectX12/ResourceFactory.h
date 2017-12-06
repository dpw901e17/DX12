#pragma once
#include "d3dx12.h"
#include "Device.h"

class ResourceFactory {

public:
	static ID3D12Resource* CreateUploadHeap(const Device& device, const int& sizeInBytes, LPCWSTR name);
	static ID3D12Resource* CreateDefaultHeap(const Device& device, const int& sizeInBytes, LPCWSTR name);
	static ID3D12Resource* CreateDefaultTextureHeap(const Device & device, D3D12_RESOURCE_DESC& textureDesc, LPCWSTR name);
	static ID3D12Resource* CreateDeafultDepthStencilHeap(const Device& device, int width, int height, LPCWSTR name);

private:
	ResourceFactory() {};
	~ResourceFactory() {};
};
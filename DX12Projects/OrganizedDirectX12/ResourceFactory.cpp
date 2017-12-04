#include "ResourceFactory.h"



ID3D12Resource* ResourceFactory::CreateUploadHeap(const Device & device, const int& sizeInBytes, LPCWSTR name)
{
	// Create upload heap
	ID3D12Resource* uploadHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&uploadHeap));
	uploadHeap->SetName(name);

	return uploadHeap;
}

ID3D12Resource* ResourceFactory::CreateDefaultHeap(const Device & device, const int& sizeInBytes, LPCWSTR name)
{
	// Create default heap
	ID3D12Resource* defaultHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // Data from upload heap will be copied to here
		nullptr,
		IID_PPV_ARGS(&defaultHeap));
	defaultHeap->SetName(name);

	return defaultHeap;
}

ID3D12Resource* ResourceFactory::CreateDefaultTextureHeap(const Device & device, D3D12_RESOURCE_DESC& textureDesc, LPCWSTR name)
{
	// Create TextureHeapDefault heap
	ID3D12Resource* defaultTextureHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&textureDesc, // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // Data from upload heap will be copied to here
		nullptr,
		IID_PPV_ARGS(&defaultTextureHeap));
	defaultTextureHeap->SetName(name);

	return defaultTextureHeap;
}

ID3D12Resource * ResourceFactory::CreateDeafultDepthStencilHeap(const Device & device, int width, int height, LPCWSTR name)
{
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ID3D12Resource* defaultDepthStencilHeap;

	// Create resource and resource heap for buffer
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&defaultDepthStencilHeap)
	);
	defaultDepthStencilHeap->SetName(name);

	return defaultDepthStencilHeap;
}

#include "ResourceFactory.h"



ID3D12Resource* ResourceFactory::CreateUploadHeap(const Device & device, const int& sizeInBytes, LPCWSTR name)
{
	ID3D12Resource* uploadHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),   
		D3D12_HEAP_FLAG_NONE,   
		&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),      
		D3D12_RESOURCE_STATE_GENERIC_READ,               
		nullptr,
		IID_PPV_ARGS(&uploadHeap));
	uploadHeap->SetName(name);

	return uploadHeap;
}

ID3D12Resource* ResourceFactory::CreateDefaultHeap(const Device & device, const int& sizeInBytes, LPCWSTR name)
{
	ID3D12Resource* defaultHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),   
		D3D12_HEAP_FLAG_NONE,   
		&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),      
		D3D12_RESOURCE_STATE_COPY_DEST,          
		nullptr,
		IID_PPV_ARGS(&defaultHeap));
	defaultHeap->SetName(name);

	return defaultHeap;
}

ID3D12Resource* ResourceFactory::CreateDefaultTextureHeap(const Device & device, D3D12_RESOURCE_DESC& textureDesc, LPCWSTR name)
{
	ID3D12Resource* defaultTextureHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),   
		D3D12_HEAP_FLAG_NONE,   
		&textureDesc,      
		D3D12_RESOURCE_STATE_COPY_DEST,          
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

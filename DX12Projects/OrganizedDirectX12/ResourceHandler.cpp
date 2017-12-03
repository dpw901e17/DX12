#include "ResourceHandler.h"

ResourceHandler::ResourceHandler(const Device& device, const resourceInfoObject& resourceInfo)
{
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(resourceInfo.heapType),
		resourceInfo.HeapFlags,
		&CD3DX12_RESOURCE_DESC::Buffer(resourceInfo.sizeInBytes),
		resourceInfo.InitialResourceState, // data from the upload heap is copied to stay here
		resourceInfo.pOptimizedClearValue,
		IID_PPV_ARGS(&resource));
	resource->SetName(resourceInfo.name);
}

ResourceHandler::~ResourceHandler()
{
	SAFE_RELEASE(resource);
}

ID3D12Resource* ResourceHandler::GetResource() const
{
	return resource;
}






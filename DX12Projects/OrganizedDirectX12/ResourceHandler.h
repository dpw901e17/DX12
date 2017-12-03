#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "Device.h"

struct resourceInfoObject {
	D3D12_HEAP_TYPE heapType;
	D3D12_HEAP_FLAGS HeapFlags;
	int  sizeInBytes;
	D3D12_RESOURCE_STATES InitialResourceState;
	D3D12_CLEAR_VALUE  *pOptimizedClearValue;
	LPCWSTR name;
};

class ResourceHandler {
public:
	ResourceHandler(const Device& device, const resourceInfoObject& resourceInfo);
	~ResourceHandler();
	ID3D12Resource* GetResource() const;

private:
	ID3D12Resource* resource;
};

class UploadHeapHandler : public ResourceHandler {
	UploadHeapHandler(int sizeInBytes);
	~UploadHeapHandler();
};
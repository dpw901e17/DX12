#include "Device.h"

Device::Device(IDXGIFactory4* dxgiFactory)
{
	HRESULT hr;
	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false;

	// find gpu supporting dx12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// No software device allowed
		//if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
		//	adapterIndex++;
		//	continue;
		//}

		// Check if hardware found. Send no interface.
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr)) {
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound) {
		std::runtime_error("No adapter found.");
	}

	// Create device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device) // Fancy macro
	);
	
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create device.");
	}
}

Device::~Device()
{
	SAFE_RELEASE(device);
}

ID3D12Device* Device::GetDevice() const
{
	return device;
}

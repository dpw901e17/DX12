#pragma once
#include "d3dx12.h"
#include <dxgi1_4.h>
#include "SafeRelease.h"
#include <iostream>

class Device {

public:
	Device(IDXGIFactory4* dxgiFactory);

	~Device();

	ID3D12Device* GetDevice() const;

private:
	ID3D12Device* device;
};
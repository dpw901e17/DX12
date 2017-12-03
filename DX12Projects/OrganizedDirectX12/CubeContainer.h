#pragma once
#include "Cube.h"
#include "d3dx12.h"
#include <d3d12.h>
#include <DirectXMath.h>

#include <vector>
#include "Device.h"
#include "../../scene-window-system/Scene.h"
#include "../../scene-window-system/Camera.h"



class CubeContainer {
public:
	CubeContainer::CubeContainer(const Device & device, int numberOfFrameBuffers, const Scene& scene);
	~CubeContainer();

	void UpdateFrame(int frameIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress(int cubeIndex, int frameBufferIndex);
	

private:
	std::vector<ID3D12Resource*> uploadHeapResources;
	std::vector<Cube> cubes;
	int constantBufferPerObjectAllignedSize = sizeof(DirectX::XMFLOAT4X4) + 255 & ~255;

	DirectX::XMFLOAT4X4 CreateProjectionMatrix(Camera cam);
	DirectX::XMFLOAT4X4 CreateViewMatrix(Camera cam);
};
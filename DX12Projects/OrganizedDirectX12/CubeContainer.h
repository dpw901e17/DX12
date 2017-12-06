#pragma once
#include "Cube.h"
#include "d3dx12.h"
#include <d3d12.h>
#include <DirectXMath.h>

#include <vector>
#include "Device.h"
#include "ResourceFactory.h"
#include "../../scene-window-system/Scene.h"
#include "../../scene-window-system/Camera.h"


class CubeContainer {
public:
	CubeContainer(const Device & device, int numberOfFrameBuffers, const Scene& scene);
	CubeContainer(const CubeContainer& cubeContainer, const size_t startIndex, const size_t count);
	~CubeContainer();

	void UpdateFrame(int frameIndex);
	D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress(int cubeIndex, int frameBufferIndex) const;
	const std::vector<ID3D12Resource*> GetUploadHeapResources() const;
	const DirectX::XMFLOAT4X4 GetProjectionMatrix() const;
	const DirectX::XMFLOAT4X4 GetViewMatrix() const;
	const std::vector<Cube> GetCubes() const;


private:
	std::vector<ID3D12Resource*> uploadHeapResources;
	std::vector<Cube> cubes;
	int constantBufferPerObjectAllignedSize = sizeof(DirectX::XMFLOAT4X4) + 255 & ~255;

	DirectX::XMFLOAT4X4 CreateProjectionMatrix(Camera cam);
	DirectX::XMFLOAT4X4 CreateViewMatrix(Camera cam);
	DirectX::XMFLOAT4X4 cameraViewMat;
	DirectX::XMFLOAT4X4 cameraProjMat;

};
#include "CubeContainer.h"

CubeContainer::CubeContainer(const Device & device, int numberOfFrameBuffers, const Scene& scene)
{
	HRESULT hr;
	auto numberOfCubes = scene.renderObjects().size();
	
	for (int i = 0; i < numberOfFrameBuffers; ++i) {
		// create resource for cube(s)
		ID3D12Resource* tmp;
		hr = device.GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferPerObjectAllignedSize * numberOfCubes), // 256 byte alligned. But also multiple of 64KB??
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, // no optimized clear value
			IID_PPV_ARGS(&tmp)
		);
		tmp->SetName(L"Constant Buffer Upload Resource Heap");
		uploadHeapResources.push_back(tmp);
	}
	auto cameraViewMat = CreateViewMatrix(scene.camera());
	auto cameraProjMat = CreateProjectionMatrix(scene.camera());

	// Initialize all cubes with an index
	for (auto i = 0; i < numberOfCubes; ++i) {
		cubes.push_back(Cube(i, uploadHeapResources, cameraProjMat, cameraViewMat, scene.renderObjects()[i]));
	}
}

CubeContainer::~CubeContainer()
{
	// Clean up uploadheaps
}

void CubeContainer::UpdateFrame(int frameIndex)
{
	for (auto i = 0; i < cubes.size(); ++i) {
		cubes[i].UpdateWVPMatrix(frameIndex);
	}
}

D3D12_GPU_VIRTUAL_ADDRESS CubeContainer::GetVirtualAddress(int cubeIndex, int frameBufferIndex)
{
	return cubes[cubeIndex].GetVirtualGpuAddress(frameBufferIndex);
}

DirectX::XMFLOAT4X4 CubeContainer::CreateProjectionMatrix(Camera cam)
{
	DirectX::XMFLOAT4X4 cameraProjMat;
	DirectX::XMMATRIX tmpMat = DirectX::XMMatrixPerspectiveFovLH(cam.FieldOfView(), cam.AspectRatio(), cam.Near(), cam.Far());
	DirectX::XMStoreFloat4x4(&cameraProjMat, tmpMat);
	return cameraProjMat;
}

DirectX::XMFLOAT4X4 CubeContainer::CreateViewMatrix(Camera cam)
{
	DirectX::XMFLOAT4X4 cameraViewMat;

	const Vec4f& camPos = cam.Position();
	auto cameraPosition = DirectX::XMFLOAT4(camPos.x, camPos.y, camPos.z, camPos.w);

	const Vec4f& camTarget = cam.Target();
	auto cameraTarget = DirectX::XMFLOAT4(camTarget.x, camTarget.y, camTarget.z, camTarget.w);

	const Vec4f& camUp = cam.Up();
	auto cameraUp = DirectX::XMFLOAT4(camUp.x, camUp.y, camUp.z, camUp.w);

	// build view matrix
	DirectX::XMVECTOR cPos = DirectX::XMLoadFloat4(&cameraPosition);
	DirectX::XMVECTOR cTarg = DirectX::XMLoadFloat4(&cameraTarget);
	DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&cameraUp);
	auto tmpMat = DirectX::XMMatrixLookAtLH(cPos, cTarg, cUp);
	DirectX::XMStoreFloat4x4(&cameraViewMat, tmpMat);

	return cameraViewMat;

}



		


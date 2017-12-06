#include "CubeContainer.h"

CubeContainer::CubeContainer(const Device & device, int numberOfFrameBuffers, const Scene& scene)
{
	HRESULT hr;
	auto numberOfCubes = scene.renderObjects().size();
	auto sizeInBytes = constantBufferPerObjectAllignedSize * numberOfCubes;

	for (int i = 0; i < numberOfFrameBuffers; ++i) {
		ID3D12Resource* uploadHeapResource = ResourceFactory::CreateUploadHeap(device, sizeInBytes, L"Constant Buffer Upload Resource Heap");
		uploadHeapResources.push_back(uploadHeapResource);
	}

	cameraViewMat = CreateViewMatrix(scene.camera());
	cameraProjMat = CreateProjectionMatrix(scene.camera());
	for (auto i = 0; i < numberOfCubes; ++i) {
		cubes.push_back(Cube(i, uploadHeapResources, cameraProjMat, cameraViewMat, scene.renderObjects()[i]));
	}
}

CubeContainer::CubeContainer(const CubeContainer & cubeContainer, const size_t startIndex, const size_t count)
{
	for (auto i = 0; i < count; ++i) {
		cubes.push_back(cubeContainer.cubes.at(i+startIndex));
	}

	uploadHeapResources = cubeContainer.GetUploadHeapResources();
	cameraViewMat = cubeContainer.GetViewMatrix();
	cameraProjMat = cubeContainer.GetProjectionMatrix();
}

CubeContainer::~CubeContainer()
{
}

void CubeContainer::UpdateFrame(int frameIndex)
{
	for (auto i = 0; i < cubes.size(); ++i) {
		cubes[i].UpdateWVPMatrix(frameIndex);
	}
}

D3D12_GPU_VIRTUAL_ADDRESS CubeContainer::GetVirtualAddress(int cubeIndex, int frameBufferIndex) const
{
	return cubes[cubeIndex].GetVirtualGpuAddress(frameBufferIndex);
}

const std::vector<ID3D12Resource*> CubeContainer::GetUploadHeapResources() const
{
	return uploadHeapResources;
}

const DirectX::XMFLOAT4X4 CubeContainer::GetProjectionMatrix() const
{
	return cameraProjMat;
}

const DirectX::XMFLOAT4X4 CubeContainer::GetViewMatrix() const
{
	return cameraViewMat;
}

const std::vector<Cube> CubeContainer::GetCubes() const
{
	return cubes;
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
	DirectX::XMVECTOR cPos = DirectX::XMLoadFloat4(&cameraPosition);
	DirectX::XMVECTOR cTarg = DirectX::XMLoadFloat4(&cameraTarget);
	DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&cameraUp);
	auto tmpMat = DirectX::XMMatrixLookAtLH(cPos, cTarg, cUp);
	DirectX::XMStoreFloat4x4(&cameraViewMat, tmpMat);

	return cameraViewMat;

}



		


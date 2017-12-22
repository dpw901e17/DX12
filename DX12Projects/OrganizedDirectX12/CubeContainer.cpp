#include "CubeContainer.h"
#include <math.h>

CubeContainer::CubeContainer(const Device & device, int numberOfFrameBuffers, const Scene& scene, float aspectRatio, ID3D12GraphicsCommandList* commandList)
{
	HRESULT hr;
	auto allignment = constantBufferPerObjectAllignedSize;
	auto numberOfCubes = scene.renderObjects().size();
	auto sizeInBytes = allignment * numberOfCubes;

	for (int i = 0; i < numberOfFrameBuffers; ++i) {
		// create resource for cube(s)
		ID3D12Resource* uploadHeapResource = ResourceFactory::CreateUploadHeap(device, sizeInBytes, L"Constant Buffer Upload Resource Heap");
		uploadHeapResources.push_back(uploadHeapResource);
	}

	// Initialize all cubes with an index
	auto dimSize = std::cbrt(scene.renderObjects().size());
	auto scale = dimSize * dimSize;
	for (auto i = 0; i < numberOfCubes; ++i) {
		cubes.push_back(Cube(i, uploadHeapResources, scene.renderObjects()[i], 1));
	}


	
	cameraViewMat = CreateViewMatrix(scene.camera());
	cameraProjMat = CreateProjectionMatrix(scene.camera(), aspectRatio);


	// Insert matrixes into byte array
	auto matSize = sizeof(DirectX::XMFLOAT4X4);
	BYTE* viewBytes = reinterpret_cast<BYTE*>(&cameraViewMat);
	BYTE* projBytes = reinterpret_cast<BYTE*>(&cameraProjMat);
	BYTE bytes[(sizeof(DirectX::XMFLOAT4X4) + 255 & ~255) * 2] = {};
	memcpy(bytes, viewBytes, matSize);
	memcpy(bytes + allignment, projBytes, matSize);

	// Make subresource
	D3D12_SUBRESOURCE_DATA matData = {};
	matData.pData = bytes; 

	// Move data into default heap
	ID3D12Resource* uploadHeap = ResourceFactory::CreateUploadHeap(device, allignment * 2, L"Constant Buffer Upload Resource Heap");
	ID3D12Resource* defaultHeap = ResourceFactory::CreateDefaultHeap(device, allignment * 2, L"Constant Buffer Default Resource Heap");
	UpdateSubresources(commandList, defaultHeap, uploadHeap, 0, 0, 1, &matData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultHeap, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// Create pointers into default heap
	viewVirtualAddress = defaultHeap->GetGPUVirtualAddress();
	projVirtualAddress = defaultHeap->GetGPUVirtualAddress() + allignment;
}

CubeContainer::CubeContainer(const CubeContainer & cubeContainer, const size_t startIndex, const size_t count)
{
	cubes.reserve(count);
	for (auto i = 0; i < count; ++i) {
		cubes.push_back(cubeContainer.cubes[i+startIndex]);
	}

	uploadHeapResources = cubeContainer.GetUploadHeapResources();
	cameraViewMat = cubeContainer.GetViewMatrix();
	cameraProjMat = cubeContainer.GetProjectionMatrix();
	viewVirtualAddress = cubeContainer.GetViewMatVirtualAddress();
	projVirtualAddress = cubeContainer.GetProjMatVirtualAddress();
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

D3D12_GPU_VIRTUAL_ADDRESS CubeContainer::GetWorldMatVirtualAddress(int cubeIndex, int frameBufferIndex) const
{
	return cubes[cubeIndex].GetVirtualGpuAddress(frameBufferIndex);
}

D3D12_GPU_VIRTUAL_ADDRESS CubeContainer::GetViewMatVirtualAddress() const
{
	return  viewVirtualAddress;
}

D3D12_GPU_VIRTUAL_ADDRESS CubeContainer::GetProjMatVirtualAddress() const
{
	return  projVirtualAddress;
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

const std::vector<Cube>& CubeContainer::GetCubes() const
{
	return cubes;
}

DirectX::XMFLOAT4X4 CubeContainer::CreateProjectionMatrix(Camera cam, float aspectRatio)
{
	DirectX::XMFLOAT4X4 cameraProjMat;
	DirectX::XMMATRIX tmpMat = DirectX::XMMatrixPerspectiveFovLH(cam.FieldOfView(), aspectRatio, cam.Near(), cam.Far());
	DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(tmpMat);
	DirectX::XMStoreFloat4x4(&cameraProjMat, transposed);
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
	DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(tmpMat);
	DirectX::XMStoreFloat4x4(&cameraViewMat, transposed);

	return cameraViewMat;

}



		


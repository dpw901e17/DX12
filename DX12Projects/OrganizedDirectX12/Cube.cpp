#include "Cube.h"
#include "../../scene-window-system/TestConfiguration.h"

Cube::Cube(int i, std::vector<ID3D12Resource*> uploadHeapResources,
	const DirectX::XMFLOAT4X4& cameraProj, const DirectX::XMFLOAT4X4& cameraView, 
	const RenderObject& renderObject, const float scale) : m_Scale(scale)
{
	HRESULT hr;

	index = i;
	auto frameBufferCount = uploadHeapResources.size();

	for (auto i = 0; i < frameBufferCount; ++i) {

		UINT8* baseGpuAddress;
		CD3DX12_RANGE readRange(0, 0);
		hr = uploadHeapResources[i]->Map(0, &readRange, reinterpret_cast<void**>(&baseGpuAddress));

		auto mappedGpuAddress = baseGpuAddress + constantBufferPerObjectAllignedSize * index;
		mappedGpuAddresses.push_back(mappedGpuAddress);

		ZeroMemory(&cubeWorldMat, sizeof(cubeWorldMat));

		memcpy(mappedGpuAddress, &cubeWorldMat, sizeof(cubeWorldMat));

		auto virtualGpuAdress = uploadHeapResources[i]->GetGPUVirtualAddress() + constantBufferPerObjectAllignedSize * index;
		virtualGpuAdresses.push_back(virtualGpuAdress);
	}

	cameraProjMat = cameraProj;
	cameraViewMat = cameraView;

	DirectX::XMStoreFloat4x4(&cubeRotMat, DirectX::XMMatrixIdentity());

	cubePosition = DirectX::XMFLOAT4(renderObject.x(), renderObject.y(), renderObject.z(), 1.0f);

}

Cube::~Cube()
{
}

UINT8 * Cube::GetMappedGpuAddress(int frameBufferIndex) const
{
	return mappedGpuAddresses[frameBufferIndex];
}

D3D12_GPU_VIRTUAL_ADDRESS Cube::GetVirtualGpuAddress(int frameBufferIndex) const
{
	return virtualGpuAdresses[frameBufferIndex];
}

void Cube::UpdateWVPMatrix(int frameBufferIndex)
{
	DirectX::XMMATRIX rotXMat = DirectX::XMMatrixRotationX(0.0001f*(index + 1) * std::pow(-1, index));
	DirectX::XMMATRIX rotYMat = DirectX::XMMatrixRotationY(0.0002f*(index + 1) * std::pow(-1, index));
	DirectX::XMMATRIX rotZMat = DirectX::XMMatrixRotationZ(0.0003f*(index + 1) * std::pow(-1, index));

	DirectX::XMMATRIX  rotMat;
	if (TestConfiguration::GetInstance().rotateCubes) {
		rotMat = DirectX::XMLoadFloat4x4(&cubeRotMat)  * rotXMat * rotYMat * rotZMat;
	}
	else {
		rotMat = DirectX::XMLoadFloat4x4(&cubeRotMat);;
	}
	DirectX::XMStoreFloat4x4(&cubeRotMat, rotMat);

	DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslationFromVector(XMLoadFloat4(&cubePosition));
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(m_Scale, m_Scale, m_Scale);

	DirectX::XMMATRIX worldMat = rotMat * translationMat * scaleMatrix;
	DirectX::XMStoreFloat4x4(&cubeWorldMat, worldMat);

	DirectX::XMMATRIX viewMat = DirectX::XMLoadFloat4x4(&cameraViewMat);
	DirectX::XMMATRIX projMat = DirectX::XMLoadFloat4x4(&cameraProjMat);
	DirectX::XMMATRIX wvpMat = DirectX::XMLoadFloat4x4(&cubeWorldMat) * viewMat * projMat;
	DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(wvpMat);
	DirectX::XMStoreFloat4x4(&cubeWorldMat, transposed);

	memcpy(mappedGpuAddresses[frameBufferIndex], &cubeWorldMat, sizeof(cubeWorldMat));
}


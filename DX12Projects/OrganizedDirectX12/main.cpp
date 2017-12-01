#include "stdafx.h"
#include <iostream>
#include  "../../scene-window-system/Window.h"
#include "../../scene-window-system/Camera.h"
#include "../../scene-window-system/RenderObject.h"
#include "../../scene-window-system/Scene.h"

using namespace DirectX;

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	Window* win = new Window(hInstance, WindowTitle, WindowName, nShowCmd, Height, Width, FullScreen);
	hwnd = win->GetHandle();

	auto tempScene = Scene(Camera::Default(), { RenderObject(0, 0, 0),
		RenderObject(0.5, 0.5, 0.5),
		RenderObject(0.5, -0.5, 0.5),
		RenderObject(-0.5, 0.5, 0.5),
		RenderObject(-0.5, -0.5, 0.5) });
	basicBoxScene = &tempScene;

	InitD3D();


	PlaySound("wh.wav", NULL, SND_FILENAME | SND_ASYNC);


	mainloop();

	//Cleanup gpu.
	WaitForPreviousFrame();
	CloseHandle(fenceEvent);

	return 0;
}


//Mainloop keeps an eye out if we are recieving
//any message from the callback function. If we are
//break the loop and display the message
//if we do not, keep running the game.
void mainloop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (Running)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//run gamecode
			++numOfFrames;
			Update();
			Render();
		}
	}
}


IDXGIFactory4* CreateDXGIFactory() {
	// -- Create the Device -- //
	HRESULT hr;
	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr)) {
		throw std::runtime_error("Could not create DXGIFactory1");
	}
	return dxgiFactory;
}

void CreateCommandQueue(const Device& device) {
	HRESULT hr;
	// -- Create the Command Queue -- //
	// This queue is where our command lists are placed to be executed on the GPU
	D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // Describes the type of queue. Using only default values
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device.GetDevice()->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&(commandQueue)));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create command queue.");
	}
}

IDXGISwapChain3* CreateSwapChain(IDXGIFactory4* dxgiFactory, DXGI_SAMPLE_DESC sampleDesc) {
	// -- Create Swap Chain with tripple buffering -- //
	DXGI_MODE_DESC backBufferDesc = {}; // Describes our display mode
	backBufferDesc.Width = Width; // buffer width
	backBufferDesc.Height = Height; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // buffer format. rgba 32 bit.

	// Multisampling. Not really using it, but we need at least one sample from buffer to display
	

	// Swap chain description
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // pipeline render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // buffer data discarded after present
	swapChainDesc.OutputWindow = hwnd; // window handle
	swapChainDesc.SampleDesc = sampleDesc; // multi-sampling descriptor
	swapChainDesc.Windowed = !FullScreen; // Apperantly more complicated than it looks

	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(
		commandQueue,
		&swapChainDesc,
		&tempSwapChain // Swap chain interface stored here
	);

	// Cast made so that we get a swapchain3. Allowing for calls to GetCurrentBackBufferIndex
	return static_cast<IDXGISwapChain3*>(tempSwapChain);
}

void CreateRTV(const Device& device, IDXGISwapChain3* swapChain) {
	HRESULT hr;
	// -- Create the render target view(RTV) descriptor heap -- //
	// The descriptors stored here each point to the backbuffers in the swap chain
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // Heap used for RTVs
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // Not visible to shaders
	hr = device.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr)) {
		throw std::runtime_error("Create descriptor heap");
	}

	// Descriptor size can vary from GPU to GPU so we need to query for it
	rtvDescriptorSize = device.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// From the d3dx12.h file. Gets handle for first descriptor in heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create an RTV for each buffer 
	for (int i = 0; i < frameBufferCount; i++) {
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr)) {
			throw std::runtime_error("Create buffer for RTV failed.");
		}

		// "Create" RTV by binding swap chain buffer to rtv handle.
		device.GetDevice()->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

		// Increment handle to next descriptor location
		rtvHandle.Offset(1, rtvDescriptorSize);
	}
}

void CreateCommandAllocator(const Device& device) {
	HRESULT hr;
	// -- Create Command Allocator -- //
	// One allocator per backbuffer, so that we may free allocators of lists not being executed on GPU.
	// The command list associated will be direct. Not bundled.


	for (int i = 0; i < frameBufferCount; i++) {
		hr = device.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr)) {
			throw std::runtime_error("Create command allocator failed.");
		}
	}
}

void CreateCommandList(const Device& device, ID3D12CommandAllocator* commandAllocator) {
	// create the command list with the first allocator
	HRESULT hr;
	hr = device.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		std::runtime_error("Create command list failed.");
	}
}

void CreateFences(const Device& device, UINT64* fenceValues) {
	//create fence
	HRESULT hr;

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = device.GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			throw std::runtime_error("Create fence event failed");
		}
		fenceValues[i] = 0;
	}
}

HANDLE CreateFenceEvent() {
	HANDLE fEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fEvent == nullptr)
	{
		throw std::runtime_error("Could not create fence event.");
	}
	return fEvent;
}

ID3D12RootSignature*  CreateRootSignature(const Device& device) {
	//--Create root signature
	// root descriptor explaining where to find data in this root parameter
	HRESULT hr;

	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
	rootCBVDescriptor.RegisterSpace = 0;
	rootCBVDescriptor.ShaderRegister = 0;

	// create a descriptor range (descriptor table) and fill it out
	// this is a range of descriptors inside a descriptor heap
	D3D12_DESCRIPTOR_RANGE descriptorTableRanges[1]; //range is 1 for now
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; //type of the range, range is a texture.
	descriptorTableRanges[0].NumDescriptors = 1; //only one texture for the moment
	descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader register
	descriptorTableRanges[0].RegisterSpace = 0; // space 0, can be zero.
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //appends to the end of the root.

																									   //descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); //one texture, one range
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; //points to the start of the range array

	D3D12_ROOT_PARAMETER rootParameters[2]; // two root parameters now! cube matrixes and texture
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //constant buffer for view root descriptor
	rootParameters[0].Descriptor = rootCBVDescriptor; //root descriptor for root parameter
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // To use for the vertex shader.

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable = descriptorTable;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // to use for the pixel shader

																		//Static sampler
																		//For bordercolour and no mipmap.
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), // one root parameter
		rootParameters, // pointer to our array of RUDEparameters
		1, //we now have a static sampler, so no longer 0
		&sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Deny for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	// Make signature
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to serialize root signature");
	}

	// Tell device to create signature
	
	ID3D12RootSignature* rSignature;
	hr = device.GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rSignature));
	if (FAILED(hr)) {
		throw  std::runtime_error("Failed to create root signature");
	}
	return rSignature;
}


ID3D12PipelineState* CreatePipeline(const Device& device, const ShaderHandler& shaderHandler, DXGI_SAMPLE_DESC sampleDesc) {
	HRESULT hr;

	//Input layer creation
	// Defines how shaders should read from vlist .
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		// 3 position coordinates
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		// 2 texture coordinates
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	//Pipeline object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc; // Layout of the vertex buffer
	psoDesc.pRootSignature = rootSignature; // Pointer to shader accessible data
	psoDesc.VS = shaderHandler.GetVertexShaderByteCode(); // Vertex shader
	psoDesc.PS = shaderHandler.GetPixelShaderByteCode(); // Pixel shader 
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // Drawing triangles
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Format of render target
	psoDesc.SampleDesc = sampleDesc; // Type of multi-sampling to use after render eg. super sampling
	psoDesc.SampleMask = 0xffffffff;   // Mask used in multi-sampling
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // default rasterizer used
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // default blend stage used at the end of pipeline
	psoDesc.NumRenderTargets = 1; // only drawing to one target per pipeline
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // use default depth/stencil buffer
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; // format of depth/stencil buffer


	ID3D12PipelineState* pso;
	hr = device.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed in create pipeline");
	}

	return pso;
}

D3D12_VERTEX_BUFFER_VIEW CreateVertexBuffer(const Device& device, ID3D12GraphicsCommandList* cList) {
	// a quad
	

	int vBufferSize = sizeof(vList);

	// Create default heap
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST, // data from the upload heap is copied to stay here
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));
	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// Create upload heap
	ID3D12Resource* vBufferUploadHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data. 
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data
										 // RowPitch = SlicePitch, because we are working with a flat buffer resource. No need to group data in another way.
										 // If using mipmaps, the row would be a texture at a mip map level and slice would go through several mipmap levels.

										 // we are now creating a command with the command list to copy the data from
										 // the upload heap to the default heap
	UpdateSubresources(cList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	cList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// create a vertex buffer view for the cube. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
	D3D12_VERTEX_BUFFER_VIEW vBufferView;
	vBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vBufferView.StrideInBytes = sizeof(Vertex);
	vBufferView.SizeInBytes = sizeof(vList);
	return vBufferView;
}

D3D12_INDEX_BUFFER_VIEW CreateIndexBuffer(const Device& device, ID3D12GraphicsCommandList* cList) {

	int iBufferSize = sizeof(iList); // 144 bytes
	numCubeIndices = sizeof(iList) / sizeof(DWORD);

	// Create default heap for holding index buffer
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST, // Initial state
		nullptr,
		IID_PPV_ARGS(&indexBuffer)
	);
	indexBuffer->SetName(L"Index Buffer Resource Heap");

	// Create upload heap for holding index buffer
	ID3D12Resource* iBufferUploadHeap;
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vList)),
		D3D12_RESOURCE_STATE_GENERIC_READ, // Initial state
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	//Store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(iList); // Pointer to index array
	indexData.RowPitch = iBufferSize; // Size of index buffer
	indexData.SlicePitch = iBufferSize; // Size of index buffer

									//Create the copying command from upload to default heap
	UpdateSubresources(commandList, indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// create a index buffer view for the triangle
	
	D3D12_INDEX_BUFFER_VIEW iBufferView;
	iBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	iBufferView.Format = DXGI_FORMAT_R32_UINT;
	iBufferView.SizeInBytes = iBufferSize;

	return iBufferView;
}

void CreateStencilBuffer(const Device& device) {
	HRESULT hr;
	// Create the depth/stencil descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device.GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create descriptor heap for stencil");
	}

	// Creating depth/stencil buffer
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	//Set clear value for depth buffer
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	// Create resource and resource heap for buffer
	device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	);
	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	//Create view of depth/stencil buffer
	device.GetDevice()->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void CreateResources(std::vector<ConstantBufferPerObject>* cBuffers, ID3D12Resource** cBufferUploadHeaps, int CBufferPerObjectAllignedSize, const Device& device) {
	//numOfCubes and basicBoxScene are globals
	HRESULT hr;

	// Creating resources for each framebuffer
	for (int i = 0; i < frameBufferCount; ++i) {
		auto numOfCubes = basicBoxScene->renderObjects().size();
		for (auto j = 0; j < numOfCubes; ++j) {
			cBuffers[i].push_back(ConstantBufferPerObject());
		}

		// create resource for cube(s)
		hr = device.GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(CBufferPerObjectAllignedSize * numOfCubes), // 256 byte alligned. But also multiple of 64KB??
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, // no optimized clear value
			IID_PPV_ARGS(&cBufferUploadHeaps[i])
		);
		cBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

		CD3DX12_RANGE readRange(0, 0); // Don't intend to read from this resource on CPU
									   // Map resource to gpu virtual address
		hr = cBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i]));

		for (auto j = 0; j < numOfCubes; ++j) {
			//constantBuffers[i] is a vector and j is the index into that vector
			auto cbPerObject = cBuffers[i][j];

			// Initialize resources to 0s.
			ZeroMemory(&cbPerObject, sizeof(cbPerObject));

			// Remember to 256 bit allign these mem copies of constant buffer!
			memcpy(cbvGPUAddress[i] + CBufferPerObjectAllignedSize * j, &cbPerObject, sizeof(cbPerObject)); // matrix 
		}
	}
}

void CreateTexture(const Device& device, ID3D12GraphicsCommandList* cList) {
	//Load texture from file
	HRESULT hr;

	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	BYTE* imageData;

	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, "box.jpg", imageBytesPerRow);

	//check if there is data in imageSize;
	if (imageSize <= 0)
	{
		std::runtime_error("Image wasn't loaded.");
	}

	hr = device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&textureBuffer));
	if (FAILED(hr))
	{
		std::runtime_error("Failed in creating default heap");
	}
	textureBuffer->SetName(L"Texture Buffer Resource Heap");

	//Get upload buffersize.
	UINT64 textureUploadBufferSize;;

	// Get size of upload heap as a multiplex of 256. Tho the last row is not restricted by this requirement.
	device.GetDevice()->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	//create upload heap to upload the texture to the GPU
	hr = device.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureBufferUploadHeap));

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create upload heap.");
	}
	textureBufferUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

	//Store texture in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = imageData;
	textureData.RowPitch = imageBytesPerRow;
	textureData.SlicePitch = imageBytesPerRow * textureDesc.Height;

	//copy upload buffer to default heap
	UpdateSubresources(cList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);

	//transition texture deafult heap to pixel shader resource in order to get colours of the pixels.
	cList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	//create descriptor heap to store srv
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = device.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to descriptor heap");
	}

	//shader resource view. Allows the shader to see the texture.
	// **adds more information about the resource than a description **
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device.GetDevice()->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//Delete imageData to free up the heap. It's uploaded to the GPU and no longer needed.
	delete imageData;
}

void CreateCubeMatrices(std::vector<CubeMatrices>& cubeMats) {
	// build projection and view matrix
	const Camera& cam = basicBoxScene->camera();


	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(cam.FieldOfView(), cam.AspectRatio(), cam.Near(), cam.Far());
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	// set starting camera state
	const Vec4f& camPos = cam.Position();
	cameraPosition = XMFLOAT4(camPos.x, camPos.y, camPos.z, camPos.w);

	const Vec4f& camTarget = cam.Target();
	cameraTarget = XMFLOAT4(camTarget.x, camTarget.y, camTarget.z, camTarget.w);

	const Vec4f& camUp = cam.Up();
	cameraUp = XMFLOAT4(camUp.x, camUp.y, camUp.z, camUp.w);

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);

	for (auto cube : basicBoxScene->renderObjects()) {

		auto cubeMat = CubeMatrices();
		cubeMat.cubePosition = XMFLOAT4(cube.x(), cube.y(), cube.z(), 0.0f);
		XMVECTOR posVec = XMLoadFloat4(&cubeMat.cubePosition);
		tmpMat = XMMatrixTranslationFromVector(posVec);

		XMStoreFloat4x4(&cubeMat.cubeRotMat, XMMatrixIdentity());

		XMStoreFloat4x4(&cubeMat.cubeWorldMat, tmpMat);

		cubeMats.push_back(cubeMat);
	}
}

void InitD3D() {
	HRESULT hr;
	
	IDXGIFactory4* dxgiFactory = CreateDXGIFactory();
	Device* device = new Device(dxgiFactory);
	CreateCommandQueue(*device);

	// Used both in swapchain and pipeline
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	swapChain = CreateSwapChain(dxgiFactory,sampleDesc);
	frameIndex = swapChain->GetCurrentBackBufferIndex();
	CreateRTV(*device, swapChain);
	CreateCommandAllocator(*device);
	CreateCommandList(*device, commandAllocator[0]);
	CreateFences(*device, fenceValue);
	fenceEvent = CreateFenceEvent();
	rootSignature = CreateRootSignature(*device);

	ShaderHandler* shaderHandler = new ShaderHandler(L"VertexShader.hlsl", L"PixelShader.hlsl");

	pipelineStateObject = CreatePipeline(*device, *shaderHandler, sampleDesc);
	vertexBufferView = CreateVertexBuffer(*device, commandList);
	indexBufferView = CreateIndexBuffer(*device, commandList);
	CreateStencilBuffer(*device);

	CreateResources(constantBuffers, constantBufferUploadHeaps, ConstantBufferPerObjectAllignedSize, *device);
	CreateTexture(*device, commandList);

	// Now we execute the command list to upload the initial assets (cube data)
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue[frameIndex]++;
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		std::runtime_error("Failed to add signal to commandqueue");
	}
	
	// Fill out the Viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Width;
	scissorRect.bottom = Height;

	CreateCubeMatrices(cubeMatrices);

	//setting globals
	globalDevice = device;
}

void Update()
{
	// Does this overwrite matrixes before they are read by the GPU?

	for (auto i = 0; i < constantBuffers[frameIndex].size(); ++i) {


		auto &cubePosition = cubeMatrices[i].cubePosition;
		auto &cubeRotMat = cubeMatrices[i].cubeRotMat;
		auto &cubeWorldMat = cubeMatrices[i].cubeWorldMat;
		auto &cbPerObject = constantBuffers[frameIndex][i];

		// rotation matrices
		XMMATRIX rotXMat = XMMatrixRotationX(0.0001f*(i+1) * std::pow(-1, i));
		XMMATRIX rotYMat = XMMatrixRotationY(0.0002f*(i+1) * std::pow(-1, i));
		XMMATRIX rotZMat = XMMatrixRotationZ(0.0003f*(i+1) * std::pow(-1, i));

		// add rotation to cube1's rot matrix
		XMMATRIX rotMat = XMLoadFloat4x4(&cubeRotMat) * rotXMat * rotYMat * rotZMat;
		XMStoreFloat4x4(&cubeRotMat, rotMat);

		// translation matrix for cube 1
		XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cubePosition));

		// world matrix for cube 1
		XMMATRIX worldMat = rotMat * translationMat;
		XMStoreFloat4x4(&cubeWorldMat, worldMat);

		// create wvp matrix
		XMMATRIX viewMat = XMLoadFloat4x4(&cameraViewMat);
		XMMATRIX projMat = XMLoadFloat4x4(&cameraProjMat);
		XMMATRIX wvpMat = XMLoadFloat4x4(&cubeWorldMat) * viewMat * projMat;
		XMMATRIX transposed = XMMatrixTranspose(wvpMat);
		XMStoreFloat4x4(&cbPerObject.wvpMat, transposed);

		// load matrix into GPU
		memcpy(cbvGPUAddress[frameIndex] + ConstantBufferPerObjectAllignedSize * i, &cbPerObject, sizeof(cbPerObject));
	}

}

void UpdatePipeline()
{
	HRESULT hr;

	WaitForPreviousFrame();
	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr))
	{
		Running = false;
	}

	// Reset of commands at the GPU and setting of the PSO
	// Make ready for recording.
	hr = commandList->Reset(commandAllocator[frameIndex], pipelineStateObject);
	if (FAILED(hr))
	{
		Running = false;
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	// get handle to depth/stencil buffer
	auto dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Sets destination of output merger.
	// Also sets the depth/stencil buffer for OM use. 
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Clears screen
	const float clearColor[] = { 1.0f, 0.5f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	commandList->SetGraphicsRootSignature(rootSignature);

	//set descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	//set descriptor table index 1 of the root signature. (corresponding to parameter order defined in the signature)
	commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// draw triangle
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// Connects the rootsignature parameter at index 0 with the constant buffer containing the wvp matrix
	// Actual draw calls
	for (auto i = 0; i < constantBuffers[frameIndex].size(); ++i) {
		commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAllignedSize * i);
		commandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);
	}

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = false;
	}


	// Right now we are using two matrixes in GPU memory at once
	// Could have just one, but would require the use of UpdateSubResource calls between render commands as to update the matrix contents.
}

void Render()
{
	HRESULT hr;

	UpdatePipeline();

	ID3D12CommandList* ppCommandLists[] = { commandList };

	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);

	if (FAILED(hr))
	{
		Running = false;
	}

	hr = swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		Running = false;
	}
}

void Cleanup()
{
	for (int i = 0; i < frameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}

	BOOL fs = false;
	if (swapChain->GetFullscreenState(&fs, NULL))
		swapChain->SetFullscreenState(false, NULL);

	delete globalDevice;
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE(commandList);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};

	SAFE_RELEASE(pipelineStateObject);
	SAFE_RELEASE(rootSignature);
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);

	SAFE_RELEASE(depthStencilBuffer);
	SAFE_RELEASE(dsDescriptorHeap);

	for (int i = 0; i < frameBufferCount; ++i) {
		SAFE_RELEASE(constantBufferUploadHeaps[i]);
	}

}

void WaitForPreviousFrame()
{
	HRESULT hr;

	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
}

int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, char* filename, int &bytesPerRow)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	//fillout texture
	resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0; //0 will let runtime decide between 64k and 4mb
	resourceDescription.Width = texWidth;
	resourceDescription.Height = texHeight;
	resourceDescription.DepthOrArraySize = 1;
	resourceDescription.MipLevels = 1;
	resourceDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // We know this as we force the format during load
	resourceDescription.SampleDesc.Count = 1;
	resourceDescription.SampleDesc.Quality = 0;
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;


	bytesPerRow = texWidth * 4; // 4 is the number of bytes in the R8G8B8A8 format

	// Copy the loaded pixels into imageData
	auto imageSize = bytesPerRow * texHeight;
	*imageData = (BYTE*)malloc(imageSize);
	memcpy(*imageData, pixels, imageSize);

	stbi_image_free(pixels);

	return imageSize;
}
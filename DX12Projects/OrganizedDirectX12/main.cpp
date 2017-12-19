#include "stdafx.h"
#include <iostream>
#include <thread>
#include  "../../scene-window-system/Window.h"
#include "../../scene-window-system/Camera.h"
#include "../../scene-window-system/RenderObject.h"
#include "../../scene-window-system/Scene.h"
#include "../../scene-window-system/WmiAccess.h"
#include "../../scene-window-system/TestConfiguration.h"


using namespace DirectX;

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	Window* win = new Window(hInstance, WindowName, WindowTitle, Width, Height);
	hwnd = win->GetHandle();

	TestConfiguration& testConfig = TestConfiguration::GetInstance();
	TestConfiguration::SetTestConfiguration(lpCmdLine);

	auto cubeCountPerDim = testConfig.cubeDimension;
	auto paddingFactor = testConfig.cubePadding;

	Camera camera = Camera::Default();
	auto heightFOV = camera.FieldOfView() / win->aspectRatio();
	auto base = (cubeCountPerDim + (cubeCountPerDim - 1) * paddingFactor) / 2.0f;
	auto camDistance = base / std::tan(heightFOV / 2);
	float z = camDistance + base + camera.Near();
	
	camera.SetPosition({ 0.0f, 0.0f, z, 1.0f });
	camera.SetFar(z + base + camera.Near());
	auto tmpScene = Scene(camera, cubeCountPerDim, paddingFactor);

	basicBoxScene = &tmpScene;

	InitD3D(*win);


	DataCollection<WMIDataItem> wmiDataCollection;
	DataCollection<PipelineStatisticsDataItem> pipelineStatisticsDataCollection;

	mainloop(wmiDataCollection, pipelineStatisticsDataCollection, testConfig, win);

	WaitForPreviousFrame(*globalSwapchain);
	CloseHandle(fenceEvent);

	return 0;
}

void mainloop(DataCollection<WMIDataItem>& wmiDataCollection, DataCollection<PipelineStatisticsDataItem>& pipelineStatisticsDataCollection, TestConfiguration& testConfig, Window* window)
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	using Clock = std::chrono::high_resolution_clock;

	size_t nanoSec = 0;
	size_t probeCount = 0;
	auto lastUpdate = Clock::now();

	WMIAccessor wmiAccesor;
	_bstr_t probeProperties[] = { "Identifier", "Value", "SensorType" };

	if (testConfig.openHardwareMonitorData) {
		wmiAccesor.Connect("OpenHardwareMonitor");
	}

	int fps = 0;	           
	int oldfps = 0;	            
	size_t secondTrackerInNanoSec = 0;
	while (Running && (nanoSec / 1000000000 < testConfig.seconds) || (testConfig.seconds == 0))
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) {
				break;
			}
			else if (msg.message == WM_KEYDOWN) {
				auto c = msg.wParam;
				if (c == 82) {
					bool& rot = TestConfiguration::GetInstance().rotateCubes;
					rot = !rot;
				}
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - lastUpdate).count();
		nanoSec += delta;
		secondTrackerInNanoSec += delta;
		lastUpdate = Clock::now();

		if (secondTrackerInNanoSec > 1000000000) {
			auto title = "FPS: " + std::to_string(fps);
			window->SetTitle(title.c_str());
			secondTrackerInNanoSec %= 1000000000;
			oldfps = fps;
			fps = 0;
		}

		if (testConfig.openHardwareMonitorData && 
			nanoSec / 1000000 > probeCount * testConfig.probeInterval) 
		{
			auto items = wmiAccesor.QueryItem("sensor", probeProperties, 3);

			for (auto& item : items) {
				item.Timestamp = std::to_string(nanoSec);
				item.Id = std::to_string(probeCount);
				item.FPS = std::to_string(oldfps);
				wmiDataCollection.Add(item);
			}

			++probeCount;
		}

		Update();
		Render(*globalSwapchain, testConfig);
		++fps;
	}

	if (testConfig.pipelineStatistics) {
		for (auto i = 0; i < testConfig.drawThreadCount; ++i) {

			PipelineStatisticsDataItem item;

			item.CommandListId = force_string(i);
			item.CInvocations = force_string(globalQueryBuffer[i].CInvocations);
			item.CPrimitives = force_string(globalQueryBuffer[i].CPrimitives);
			item.CSInvocations = force_string(globalQueryBuffer[i].CSInvocations);
			item.DSInvocations = force_string(globalQueryBuffer[i].DSInvocations);
			item.GSInvocations = force_string(globalQueryBuffer[i].GSInvocations);
			item.GSPrimitives = force_string(globalQueryBuffer[i].GSPrimitives);
			item.HSInvocations = force_string(globalQueryBuffer[i].HSInvocations);
			item.IAPrimitives = force_string(globalQueryBuffer[i].IAPrimitives);
			item.IAVertices = force_string(globalQueryBuffer[i].IAVertices);
			item.PSInvocations = force_string(globalQueryBuffer[i].PSInvocations);
			item.VSInvocations = force_string(globalQueryBuffer[i].VSInvocations);

			pipelineStatisticsDataCollection.Add(item);
		}
	}

	auto now = time(NULL);
	tm* localNow = new tm();
	localtime_s(localNow, &now);

	auto yearStr = std::to_string((1900 + localNow->tm_year));
	auto monthStr = localNow->tm_mon < 9 ? "0" + std::to_string(localNow->tm_mon + 1) : std::to_string(localNow->tm_mon + 1);
	auto dayStr = localNow->tm_mday < 10 ? "0" + std::to_string(localNow->tm_mday) : std::to_string(localNow->tm_mday);
	auto hourStr = localNow->tm_hour < 10 ? "0" + std::to_string(localNow->tm_hour) : std::to_string(localNow->tm_hour);
	auto minStr = localNow->tm_min < 10 ? "0" + std::to_string(localNow->tm_min) : std::to_string(localNow->tm_min);
	auto secStr = localNow->tm_sec < 10 ? "0" + std::to_string(localNow->tm_sec) : std::to_string(localNow->tm_sec);

	auto fname = yearStr + monthStr + dayStr + hourStr + minStr + secStr;

	if (testConfig.exportCsv && testConfig.openHardwareMonitorData) {

		auto csvStr = wmiDataCollection.MakeString(";");
		SaveToFile("data_" + fname + ".csv", csvStr);
	}

	if (testConfig.exportCsv && testConfig.pipelineStatistics) {
		auto csvStr = pipelineStatisticsDataCollection.MakeString(";");
		SaveToFile("stat_" + fname + ".csv", csvStr);
	}

	if (testConfig.exportCsv) {
		auto csvStr = TestConfiguration::GetInstance().MakeString(";");
		SaveToFile("conf_" + fname + ".csv", csvStr);
	}

	delete localNow;
}

IDXGIFactory4* CreateDXGIFactory() {
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
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};          
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device.GetDevice()->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&(commandQueue)));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create command queue.");
	}
}

void CreateRTV(const Device& device, SwapChainHandler swapChainHandler) {
	HRESULT hr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;     
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;     
	hr = device.GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr)) {
		throw std::runtime_error("Create descriptor heap");
	}

	rtvDescriptorSize = device.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < frameBufferCount; i++) {
		hr = swapChainHandler.GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr)) {
			throw std::runtime_error("Create buffer for RTV failed.");
		}

		device.GetDevice()->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

		rtvHandle.Offset(1, rtvDescriptorSize);
	}
}

void CreateCommandAllocator(const Device& device) {
	HRESULT hr;
	for (int i = 0; i < frameBufferCount; i++) {
		hr = device.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr)) {
			throw std::runtime_error("Create command allocator failed.");
		}
	}
}

void CreateCommandList(const Device& device, ID3D12CommandAllocator* commandAllocator) {
	HRESULT hr;
	hr = device.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		std::runtime_error("Create command list failed.");
	}
}

void CreateFences(const Device& device, UINT64* fenceValues) {
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
	HRESULT hr;

	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
	rootCBVDescriptor.RegisterSpace = 0;
	rootCBVDescriptor.ShaderRegister = 0;

	D3D12_DESCRIPTOR_RANGE descriptorTableRanges[1];     
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;        
	descriptorTableRanges[0].NumDescriptors = 1;      
	descriptorTableRanges[0].BaseShaderRegister = 0;       
	descriptorTableRanges[0].RegisterSpace = 0;      
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;       

	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges);    
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0];        

	D3D12_ROOT_PARAMETER rootParameters[2];         
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;      
	rootParameters[0].Descriptor = rootCBVDescriptor;     
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;       

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable = descriptorTable;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;       

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
	rootSignatureDesc.Init(_countof(rootParameters),    
		rootParameters,       
		1,          
		&sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |     
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to serialize root signature");
	}

	ID3D12RootSignature* rSignature;
	hr = device.GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rSignature));
	if (FAILED(hr)) {
		throw  std::runtime_error("Failed to create root signature");
	}
	return rSignature;
}

D3D12_INPUT_LAYOUT_DESC  inputLayout() {
	auto inputLayout = new D3D12_INPUT_ELEMENT_DESC[2];
	inputLayout[0] = D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayout[1] = D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.NumElements = 2;
	inputLayoutDesc.pInputElementDescs = inputLayout;

	return inputLayoutDesc;
}

D3D12_VERTEX_BUFFER_VIEW CreateVertexBuffer(const Device& device, ID3D12GraphicsCommandList* cList) {
	int vBufferSize = sizeof(vList);

	ID3D12Resource* vertexBuffer = ResourceFactory::CreateDefaultHeap(device, vBufferSize, L"Default Heap Vertex Buffer");
	ID3D12Resource* vBufferUploadHeap = ResourceFactory::CreateUploadHeap(device, vBufferSize, L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList);      
	vertexData.RowPitch = vBufferSize;         
	vertexData.SlicePitch = vBufferSize;         
	UpdateSubresources(cList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	cList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	D3D12_VERTEX_BUFFER_VIEW vBufferView;
	vBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vBufferView.StrideInBytes = sizeof(Vertex);
	vBufferView.SizeInBytes = sizeof(vList);
	return vBufferView;
}

D3D12_INDEX_BUFFER_VIEW CreateIndexBuffer(const Device& device, ID3D12GraphicsCommandList* cList) {

	int iBufferSize = sizeof(iList);   
	numCubeIndices = sizeof(iList) / sizeof(DWORD);

	ID3D12Resource* indexBuffer = ResourceFactory::CreateDefaultHeap(device, iBufferSize, L"Index Buffer Resource Heap");
	ID3D12Resource* iBufferUploadHeap = ResourceFactory::CreateUploadHeap(device, iBufferSize, L"Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(iList);     
	indexData.RowPitch = iBufferSize;     
	indexData.SlicePitch = iBufferSize;     
	
	UpdateSubresources(commandList, indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	D3D12_INDEX_BUFFER_VIEW iBufferView;
	iBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	iBufferView.Format = DXGI_FORMAT_R32_UINT;
	iBufferView.SizeInBytes = iBufferSize;

	return iBufferView;
}

void CreateStencilBuffer(const Device& device) {
	HRESULT hr;
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device.GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create descriptor heap for stencil");
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	ID3D12Resource* depthStencilBuffer = ResourceFactory::CreateDeafultDepthStencilHeap(device, Width, Height, L"Depth/Stencil Resource Heap");

	device.GetDevice()->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void CreateTexture(const Device& device, ID3D12GraphicsCommandList* cList) {
	HRESULT hr;

	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	BYTE* imageData;

	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, "box.jpg", imageBytesPerRow);

	if (imageSize <= 0)
	{
		std::runtime_error("Image wasn't loaded.");
	}

	ID3D12Resource* textureBuffer = ResourceFactory::CreateDefaultTextureHeap(device, textureDesc, L"Texture Buffer Resource Heap");

	UINT64 textureUploadBufferSize;

	device.GetDevice()->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	ID3D12Resource* textureBufferUploadHeap = ResourceFactory::CreateUploadHeap(device, textureUploadBufferSize, L"Texture Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = imageData;
	textureData.RowPitch = imageBytesPerRow;
	textureData.SlicePitch = imageBytesPerRow * textureDesc.Height;

	UpdateSubresources(cList, textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);

	cList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;    
	hr = device.GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create descriptor heap");
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device.GetDevice()->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	delete imageData;
}

void InitD3D(Window window) {
	HRESULT hr;
	
	ID3D12Debug* debugController;
	ID3D12Debug1* debug1Controller;
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to initialize debug controller!");
	
	}
	else if (SUCCEEDED(hr)) {
		debugController->EnableDebugLayer();
		debugController->QueryInterface(IID_PPV_ARGS(&debug1Controller));
		debug1Controller->SetEnableGPUBasedValidation(true);
	}

	IDXGIFactory4* dxgiFactory = CreateDXGIFactory();
	Device* device = new Device(dxgiFactory);
	CreateCommandQueue(*device);

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	SwapChainHandler* swapChainHandler = new SwapChainHandler(dxgiFactory, sampleDesc, frameBufferCount, window, commandQueue);

	frameIndex = swapChainHandler->GetSwapChain()->GetCurrentBackBufferIndex();
	CreateRTV(*device, *swapChainHandler);
	CreateCommandAllocator(*device);
	CreateCommandList(*device, commandAllocator[0]);
	CreateFences(*device, fenceValue);
	fenceEvent = CreateFenceEvent();
	rootSignature = CreateRootSignature(*device);

	ShaderHandler* shaderHandler = new ShaderHandler(L"VertexShader.hlsl", L"PixelShader.hlsl");
	ShaderHandler* shaderHandler2 = new ShaderHandler(L"VertexShader.hlsl", L"RedPixelShader.hlsl");

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = inputLayout();
	globalPipeline = new PipelineStateHandler(*device, *shaderHandler, inputLayoutDesc, sampleDesc, *rootSignature);
	globalPipeline2 = new PipelineStateHandler(*device, *shaderHandler2, inputLayoutDesc, sampleDesc, *rootSignature);


	vertexBufferView = CreateVertexBuffer(*device, commandList);
	indexBufferView = CreateIndexBuffer(*device, commandList);
	CreateStencilBuffer(*device);

	globalCubeContainer = new CubeContainer(*device, frameBufferCount, *basicBoxScene, window.aspectRatio());

	CreateTexture(*device, commandList);

	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	fenceValue[frameIndex]++;
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		std::runtime_error("Failed to add signal to commandqueue");
	}
	
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Width;
	viewport.Height = Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Width;
	scissorRect.bottom = Height;

	globalDevice = device;
	globalSwapchain = swapChainHandler;
	globalStartCommandListHandler = new CommandListHandler(*device, frameBufferCount);
	globalEndCommandListHandler = new CommandListHandler(*device, frameBufferCount);

	for (auto i = 0; i < TestConfiguration::GetInstance().drawThreadCount; i++) {
		drawCommandLists.push_back(new CommandListHandler(*device, frameBufferCount));
	}

	D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
	queryHeapDesc.Count = TestConfiguration::GetInstance().drawThreadCount;
	queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;

	if (FAILED(device->GetDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&globalQueryHeap)))) {
		throw std::runtime_error("Could not create query heap (for pipeline statistics)!");
	}

	if (FAILED(device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK), 
		D3D12_HEAP_FLAG_NONE, 
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) * TestConfiguration::GetInstance().drawThreadCount),
		D3D12_RESOURCE_STATE_COPY_DEST, 
		nullptr, 
		IID_PPV_ARGS(&globalQueryResult)))) 
	{
		throw std::runtime_error("Could not create committed ressource for pipeline statistics (query result)!");
	}

	globalThreadPool = new ThreadPool<DrawCubesInfo>(TestConfiguration::GetInstance().drawThreadCount);
}

void Update()
{
	globalCubeContainer->UpdateFrame(frameIndex);
}

void UpdatePipeline(TestConfiguration testConfig)
{
	WaitForPreviousFrame(*globalSwapchain);

	if (testConfig.pipelineStatistics) {
		D3D12_RANGE emptyRange = { 0,0 };
		D3D12_RANGE range = {};
		range.Begin = 0;
		range.End = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) * TestConfiguration::GetInstance().drawThreadCount;
		globalQueryResult->Map(0, &range, reinterpret_cast<void**>(&globalQueryBuffer));
		globalQueryResult->Unmap(0, &emptyRange);
	}

	auto cubeCount = basicBoxScene->renderObjects().size();

	auto threadCount = TestConfiguration::GetInstance().drawThreadCount;
	for (auto i = 0; i < threadCount; ++i) {

		DrawCubesInfo info = {};
		info.commandListHandler = drawCommandLists[i];

		if (i == threadCount - 1) {
			info.cubeCount = cubeCount / threadCount + cubeCount % threadCount;
		}
		else {
			info.cubeCount = cubeCount / threadCount;
		}

		info.drawStartIndex = cubeCount / threadCount * i;
		info.dsDescriptorHeap = dsDescriptorHeap;
		info.frameIndex = frameIndex;
		info.globalCubeContainer = globalCubeContainer;
		info.indexBufferView = &indexBufferView;
		info.mainDescriptorHeap = mainDescriptorHeap;
		info.numCubeIndices = numCubeIndices;
		info.pipelineStateHandler = globalPipeline;
		info.renderTargets = renderTargets;
		info.rootSignature = rootSignature;
		info.rtvDescriptorHeap = rtvDescriptorHeap;
		info.rtvDescriptorSize = rtvDescriptorSize;
		info.scissorRect = scissorRect;
		info.vertexBufferView = &vertexBufferView;
		info.viewport = viewport;
		info.queryIndex = i;

		ThreadJob<DrawCubesInfo> job = ThreadJob<DrawCubesInfo>(DrawCubes, info);
		globalThreadPool->AddThreadJob(job);
	}

	globalStartCommandListHandler->Open(frameIndex, *globalPipeline->GetPipelineStateObject());
	globalStartCommandListHandler->RecordOpen(renderTargets);
	globalStartCommandListHandler->RecordClearScreenBuffers(*rtvDescriptorHeap, rtvDescriptorSize, *dsDescriptorHeap);
	globalStartCommandListHandler->Close();

	globalEndCommandListHandler->Open(frameIndex, *globalPipeline->GetPipelineStateObject());
	globalEndCommandListHandler->RecordClosing(renderTargets, globalQueryHeap, threadCount, globalQueryResult);
	globalEndCommandListHandler->Close();

	while (!globalThreadPool->Idle())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(TEST_THREAD_JOB_WAIT_TIME));
	}

}

void DrawCubes(DrawCubesInfo& info)
{
	info.commandListHandler->Open(info.frameIndex, *info.pipelineStateHandler->GetPipelineStateObject());
	info.commandListHandler->SetState(
		info.renderTargets, 
		*info.rtvDescriptorHeap, 
		info.rtvDescriptorSize, 
		*info.dsDescriptorHeap, 
		*info.rootSignature, 
		*info.mainDescriptorHeap, 
		info.viewport, 
		info.scissorRect, 
		*info.vertexBufferView, 
		*info.indexBufferView
	);
	info.commandListHandler->RecordDrawCalls(CubeContainer(*info.globalCubeContainer, info.drawStartIndex, info.cubeCount), info.numCubeIndices, globalQueryHeap, info.queryIndex);
	info.commandListHandler->Close();
}


void Render(SwapChainHandler swapChainHandler, TestConfiguration testConfig)
{
	HRESULT hr;
	UpdatePipeline(testConfig);

	std::vector<ID3D12CommandList*> comListVec;
	comListVec.push_back(globalStartCommandListHandler->GetCommandList());

	std::stringstream queueCommands;	  
	for (auto& comList : drawCommandLists) {
		comListVec.push_back(comList->GetCommandList());

		queueCommands << "ID3D12CommandList: " << comList->GetCommandList() << "\r\n";
		queueCommands << comList->GetGPUCommandDebugString();
	}

	comListVec.push_back(globalEndCommandListHandler->GetCommandList());


	commandQueue->ExecuteCommandLists(comListVec.size(), comListVec.data());
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed in creating signal on commandQueue");
	}

	hr = swapChainHandler.GetSwapChain()->Present(0, 0);

	
	if (FAILED(hr))
	{
		hr = globalDevice->GetDevice()->GetDeviceRemovedReason();
		throw std::runtime_error("Failed to present swapchain.");
	}
}

void Cleanup(SwapChainHandler swapChainHandler)
{
	for (int i = 0; i < frameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame(*globalSwapchain);
	}

	BOOL fs = false;
	if (swapChainHandler.GetSwapChain()->GetFullscreenState(&fs, NULL))
		swapChainHandler.GetSwapChain()->SetFullscreenState(false, NULL);

	delete globalDevice;
	delete globalSwapchain;
	SAFE_RELEASE(commandQueue);

	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE(commandList);	

	SAFE_RELEASE(globalQueryResult);
	SAFE_RELEASE(globalQueryHeap);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};
	delete globalCommandListHandler;

	for (auto& comList : drawCommandLists) {
		delete comList;
	}

	delete globalPipeline;
	SAFE_RELEASE(rootSignature);
	SAFE_RELEASE(indexBuffer);

	SAFE_RELEASE(depthStencilBuffer);
	SAFE_RELEASE(dsDescriptorHeap);

}

void WaitForPreviousFrame(SwapChainHandler& swapChainHandler)
{
	HRESULT hr;

	frameIndex = swapChainHandler.GetSwapChain()->GetCurrentBackBufferIndex();

	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValue[frameIndex]++;
}

int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, char* filename, int &bytesPerRow)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0;         
	resourceDescription.Width = texWidth;
	resourceDescription.Height = texHeight;
	resourceDescription.DepthOrArraySize = 1;
	resourceDescription.MipLevels = 1;
	resourceDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;           
	resourceDescription.SampleDesc.Count = 1;
	resourceDescription.SampleDesc.Quality = 0;
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;


	bytesPerRow = texWidth * 4;           

	auto imageSize = bytesPerRow * texHeight;
	*imageData = (BYTE*)malloc(imageSize);
	memcpy(*imageData, pixels, imageSize);

	stbi_image_free(pixels);

	return imageSize;
}

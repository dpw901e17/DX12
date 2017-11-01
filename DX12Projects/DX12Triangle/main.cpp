#include "stdafx.h"

#define TEST_PROBE_INTERVAL_MS 0	//<-- probe each frame

#define TEST_CSV_ARG "-csv"
#define TEST_PERFMON_ARG "-perfmon"
#define TEST_TIME_ARG "-sec"

void Cleanup() {
	// wait for gpu to finish all of its frames
	for (int i = 0; i < frameBufferCount; i++) {
		frameIndex = i;
		WaitForPreviousFrame();
	}

	// get swapchain out of fullscreen
	BOOL fs = false;
	if (swapChain->GetFullscreenState(&fs, NULL))
		swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE(commandList);
	SAFE_RELEASE(pipelineStateObject);
	SAFE_RELEASE(rootSignature);
	SAFE_RELEASE(vertexBuffer);

	for (int i = 0; i < frameBufferCount; i++) {
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	}

}

void WaitForPreviousFrame() {
	HRESULT hr;
	// swap current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// We wait if the fence has not been updated with the signal command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex]) {
		// Set up event to signal us when the fence has been updated
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr)) {
			Running = false;
		}

		WaitForSingleObject(fenceEvent, INFINITE);
	}
	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
}

bool InitD3D() {
	HRESULT hr;

	// -- Create the Device -- //
	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr)) {
		return false;
	}

	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false;

	// find gpu supporting dx12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// No software device allowed
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			adapterIndex++;
			continue;
		}

		// Check if hardware found. Send no interface.
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr)) {
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound) {
		return false;
	}

	// Create device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device) // Fancy macro
	);
	if (FAILED(hr)) {
		return false;
	}

	// -- Create the Command Queue -- //
	// This queue is where our command lists are placed to be executed on the GPU
	D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // Describes the type of queue. Using only default values
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&(commandQueue)));
	if (FAILED(hr)) {
		return false;
	}

	// -- Create Swap Chain with tripple buffering -- //
	DXGI_MODE_DESC backBufferDesc = {}; // Describes our display mode
	backBufferDesc.Width = Width; // buffer width
	backBufferDesc.Height = Height; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // buffer format. rgba 32 bit.

														// Multisampling. Not really using it, but we need at least one sample from buffer to display
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

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
	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// -- Create the render target view(RTV) descriptor heap -- //
	// The descriptors stored here each point to the backbuffers in the swap chain
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // Heap used for RTVs
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // Not visible to shaders
	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr)) {
		return false;
	}

	// Descriptor size can vary from GPU to GPU so we need to query for it
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// From the d3dx12.h file. Gets handle for first descriptor in heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create an RTV for each buffer 
	for (int i = 0; i < frameBufferCount; i++) {
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr)) {
			return false;
		}

		// "Create" RTV by binding swap chain buffer to rtv handle.
		device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

		// Increment handle to next descriptor location
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	// -- Create Command Allocator -- //
	// One allocator per backbuffer, so that we may free allocators of lists not being executed on GPU.
	// The command list associated will be direct. Not bundled.
	for (int i = 0; i < frameBufferCount; i++) {
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr)) {
			return false;
		}
	}

	// -- Create Command Lists -- //
	// One list per thread. No multithreading here so only 1.
	// As lists can be reset right after executing on queue we need only one. 
	// The fírst of our 3 allocators are set initially.
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr)) {
		return false;
	}

	// -- Create fence and fence event -- //
	// Fences. Used to lock memory such as command lists and allocators while they're in use.
	for (int i = 0; i < frameBufferCount; i++) {
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr)) {
			return false;
		}
		fenceValue[i] = 0; // Setting initial value
	}

	// Fence event handle. Triggered when fence changes and we can go on.
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr) {
		return false;
	}

	// -- Create root signature -- //
	// No parameters but activation of input assembler
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// serialize signature into bytecode 
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	if (FAILED(hr)) {
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr)) {
		return false;
	}

	// -- Shader time -- //

	// Compile vertex shader
	ID3DBlob* vertexShader;
	ID3DBlob* errorBuff;
	hr = D3DCompileFromFile(L"VertexShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShader,
		&errorBuff);
	if (FAILED(hr)) {
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	// make pointer to bytecode
	D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
	vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
	vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

	// Compile pixel shader
	ID3DBlob* pixelShader;
	hr = D3DCompileFromFile(L"PixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShader,
		&errorBuff);
	if (FAILED(hr)) {
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	// bytecode pointer
	D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
	pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

	//--Create Input layout--//
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Input layout description structure
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	//-- PSO creation ---//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.pRootSignature = rootSignature;
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;

	// creation
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject));
	if (FAILED(hr)) {
		return false;
	}

	//-- Create Vertex Buffer --//
	Vertex vList[] = {
		{ { 0.0f, 0.5f, 0.5f } },
		{ { 0.5f, -0.5f, 0.5f } },
		{ { -0.5f, -0.5f, 0.5f } }
	};

	int vBufferSize = sizeof(vList);

	//Create default heap on GPU
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST, // Initial state to copy from upload heap
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));

	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	//Create upload heap on CPU
	ID3D12Resource* vBufferUploadHeap;
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, // Initial state set to be read from
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// Store buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); // Cast to byte array
	vertexData.RowPitch = vBufferSize; // Size of triangle vertex data
	vertexData.SlicePitch = vBufferSize;

	// Create copy command
	UpdateSubresources(commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// Execute commandlist  to upload initial assets
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Increment fence value
	fenceValue[frameIndex]++;
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr)) {
		Running = false;
	}

	// make vertex buffer view for triangle
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vBufferSize;

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


	return true;
}


bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullscreen) {
	if (fullscreen)
	{
		// Handle to device monitor
		HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

		// Information about monitor
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		// Setting screen size using montior information
		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	// Initializing the windows class
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW; // Defines additional elements of class
	wc.lpfnWndProc = WndProc; // Pointer to window function
	wc.cbClsExtra = NULL; // Extra bytes to allocate
	wc.cbWndExtra = NULL; // 
	wc.hInstance = hInstance; // Handle to instance that contains windows procedure
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);  // Handle to Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Setting Cursor
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2); // Set Color
	wc.lpszMenuName = NULL; // Name of menu
	wc.lpszClassName = WindowName; // Name of window
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Error registering class", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hwnd = CreateWindowEx(
		NULL, // Extended style
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW, // Style 
		CW_USEDEFAULT, // Initial horizontal position
		CW_USEDEFAULT, // Initial Vertical position
		width, height,
		NULL, // Parent handle
		NULL, // Menu handle
		hInstance,
		NULL // Valueto be passed to the window
	);

	if (!hwnd) {
		MessageBox(NULL, "Error creating window", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen) {
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, ShowWnd); // Shows window using handle and a specification of the window
	UpdateWindow(hwnd); // Update the window 

	return true;
}

void Update() {
	// Update app logic.
}

void UpdatePipeline() {
	// This is where we add commands to the command list
	HRESULT hr;
	WaitForPreviousFrame(); // Wait for gpu to finish using allocator before reset

							// Resetting command allocator. Freeing memory command list was stored in on GPU.
	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr)) {
		Running = false;
	}

	// By resetting the commandlist we put it in recording mode.
	// The allocator may be asscoiated with several lists. Make sure only one is recording.

	hr = commandList->Reset(
		commandAllocator[frameIndex],
		pipelineStateObject // PSO. Not added yet as we're only clearing RTV
	);
	if (FAILED(hr)) {
		Running = false;
	}

	// Recording commands into the command list
	// Transition current index from present state to render target state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// -- Record start -- //
	// Get handle to current rtv
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	// Set render target to output merger state
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Clear render target
	const float clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// draw triangle
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);

	// Transition current index back to present state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	// -- Record end -- //

	hr = commandList->Close(); // This is where errors in working with commandlist might pop up
	if (FAILED(hr)) {
		Running = false;
	}
}

void Render(long long timestamp) {
	HRESULT hr;

	UpdatePipeline(); // set command lists


					  // Create array of command lists. Only 1 here since single threading
	ID3D12CommandList* ppCommandLists[] = { commandList };

	// Execute aray
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


	// Signal set at the end of queue. Sets the fence value to indicate that queue has finished.
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr)) {
		Running = false;
	}

	// present current backbuffer
	hr = swapChain->Present(0, 0);
	if (FAILED(hr)) {
		Running = false;
	}


}

void mainloop() {
	// Makes room for a message
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	bool run = true;
	using Clock = std::chrono::high_resolution_clock;
	auto lastUpdate = Clock::now();
	auto test = Clock::now() - lastUpdate;

	size_t nanoSec = 0;
	size_t probeCount = 0;	//<-- how many times we have collected data this test


	WMIAccessor wmiAccessor("OpenHardwareMonitor");
	WMIDataCollection database;
	_bstr_t probe_properties[] = { "Identifier", "Value", "SensorType" };	//<-- determined by OpenHardwareMonitor


	while (argTime > nanoSec / 1000000000) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// Exit when "X" icon is pressed
			if (msg.message == WM_QUIT)
				break;

			// Send message onwards
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//timing code
		nanoSec += std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - lastUpdate).count();
		// game code
		lastUpdate = Clock::now();
		Update();
		Render(nanoSec);

		if (argCsv) {
			if (nanoSec / 1000000 > probeCount * TEST_PROBE_INTERVAL_MS)
			{
				++probeCount;
				auto item = wmiAccessor.QueryItem("sensor", probe_properties, 3, Arrange_Test_Data);
				item.Add("Timestamp", std::to_string(nanoSec));
				database.Add(item);
			}
		}

	} //END while

	if (argCsv) {

		std::vector<std::string> order = { "Timestamp", "ComponentType", "ComponentID", "SensorType", "SensorID", "Value" };
		auto csvStr = database.MakeString(order, ";");

		//the timestamp for filename will be of the format yyyymmddhhmmss = 14 characters long
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

		SaveToFile("data_" + fname + ".csv", csvStr);
		delete localNow;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	// Catches the messages from the window and handles them
	switch (msg)
	{
		//  Send a closing messsage at "esc" press
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, "Are you sure you want exit?", "Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

		// Close Window once destroyed
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR exeArg, int nShowCmd) {

	//get exe arguments
	std::vector<std::string> args;

	std::string str(exeArg);
	std::string arg = "";
	for (char c : str) {
		if (c == ' ') {
			args.push_back(arg);
			arg = "";
		}
		else {
			arg += c;
		}
	}

	args.push_back(arg);

	std::string a = "";
	for (auto i = 0; i < args.size(); ++i) {
		a = args[i];
		if (a == TEST_CSV_ARG) {
			argCsv = true;
		}
		else if (a == TEST_PERFMON_ARG) {
			argPerfmon = true;
		}
		else if (a == TEST_TIME_ARG) {
			argTime = stoi(args[i + 1]);
		}
	}


	//END arg analysis

	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, FullScreen)) {
		MessageBox(0, "Window Initialization - Failed", "Error", MB_OK);
	}

	if (!InitD3D()) {
		MessageBox(0, "Failed to initialize direct3d 12", "Error", MB_OK);
		Cleanup();
		return 1;
	}

	mainloop();

	WaitForPreviousFrame();
	CloseHandle(fenceEvent); // Deallocate handle.
	Cleanup();

	return 0;
}

//Determines how the cells (items) in the database look (name = attribute/collumn in db, value = entry in cell)
void Arrange_Test_Data(const std::string* dataArr, WMIDataItem* item)
{
	/*
	dataArr[0] is Identifyer (when called in this main).
	dataArr[1] is Value
	dataArr[2] is SensorType

	Since Identifyer has the general structure "/[component]/[compId]/[sensorType]/[sensorId]",
	then this can be split into db collumns (parts) like:  [component] | [compId] | [sensorId]
	*/

	//split Identifyer up as described above:
	std::vector<std::string> parts;
	std::string part = "";
	for (char c : dataArr[0]) {
		if (c == '/') {
			if (part != "") {
				parts.push_back(part);
				part = "";
			}
		}
		else {
			part += c;
		}
	}

	//add the last identifyed part:
	parts.push_back(part);

	item->Add("ComponentType", parts[0]);
	//some identifiers (/ram/data/[id]) only have 3 parts to them (missing the ComponentID)
	if (parts.size() % 4 == 0) {
		item->Add("ComponentID", parts[1]);
	}
	else {
		item->Add("ComponentID", "N/A");
	}
	item->Add("SensorID", parts[parts.size() - 1]);
	item->Add("Value", dataArr[1]);
	item->Add("SensorType", dataArr[2]);
}

void SaveToFile(const std::string& file, const std::string& data)
{
	auto fname = file;
	ofstream fs;
	fs.open(fname, std::ofstream::app);
	fs << data;
	fs.close();
}
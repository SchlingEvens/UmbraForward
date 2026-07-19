#include "SampleTriangle.h"

//don't use default old dx12 core,and use statement version.
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\"; }

//
SampleTriangle::SampleTriangle(UINT weight, UINT height, std::wstring name):
	RenderAppBase(weight,height,name),
	m_frameIndex(0),
	m_viewport(0.0f,0.0f,static_cast<float>(weight),static_cast<float>(height)),
	m_scissorRect(0,0,static_cast<LONG>(weight),static_cast<LONG>(height)),
	m_rtvDescrptorSize(0)
{}

void SampleTriangle::OnInit(){
	LoadPipeline();
	LoadAssets();
}


//Frame update about data
void SampleTriangle::OnUpdate(){
}

//Frame update about render command release,the data based OnUpdate.
void SampleTriangle::OnRender()
{
	//prepare command list to render.
	PopulateCommandList();

	//execute command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();

}

void SampleTriangle::OnDestroy()
{
}


//Step 1 :pipeline init
void SampleTriangle::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;   

#if defined(_DEBUG)
	//enable debug layer before create device;
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			//enable debug layer.
			debugController->EnableDebugLayer();
			//add debug layer to factory flags.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	//create factory ptr and instantiate.
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	//m_useWarpDevice :from base class
	//use gpu(false) or cpu(true) to create device.
	if (m_useWarpDevice) {
		//create adapter
		ComPtr<IDXGIAdapter>warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		//create device and add to m_device
		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	//in a similar way
	else {
		ComPtr<IDXGIAdapter1>hardwareAdapter;
		//the function defined in base class to enumerate the hardware adapter.
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);
	
		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	//describe and create command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));


	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	//first create a lower version of swap chain
	//then promate it to IDXGISwapChain3 to use later.
	//the create function can only create IDXGISwapChain1.
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	//forbid(don't use) alt+enter fullscreen.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	//promote swap chain to IDXGISwapChain3
	ThrowIfFailed(swapChain.As(&m_swapChain));
	//mark the current back buffer index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	//create description heap for rtv.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	//mark the size of rtv descriptor.
	m_rtvDescrptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//create frame resources(swap chain's buffers) and add to m_renderTargets(array).
	//for each buffer.
	//rtvHandle:the address of render target view 's descrptor in rtvdescheap
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < FrameCount; i++) {
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescrptorSize);

		//create command allocator for each frame.
		ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])));
	}
}

//Load Assets.
void SampleTriangle::LoadAssets()
{
	//create and init root signature.
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		//The Desc means the root signature has 0 parameters, 0 static samplers, and allows input assembler input layout( use vetex buffer and index buffer). 
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		
		//Serialize the root signature desc to a blob,and create signature.
		ComPtr<ID3DBlob> signature;    
		ComPtr<ID3DBlob> error;   //The blob obj keep error info(if happened) to debug.
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	//compiling and loading shader.
	{
		ComPtr<ID3DBlob>vertexShader;
		ComPtr<ID3DBlob>pixelShader;
		ComPtr<ID3DBlob>vertexError;
		ComPtr<ID3DBlob>pixelError;

#if define(_DEBUG)
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		//loading shader from local file.
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &vertexError));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &pixelError));
	
		//Create vertex input layout.(vertex struct define in head file)
		D3D12_INPUT_ELEMENT_DESC elementDescs[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}};

		//Describe and create Pipeline State Obj (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { elementDescs,_countof(elementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	//create command list;
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator->Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	//create fence and sync object.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
		if (m_fenceEvent == nullptr) {
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		
	}

	//Create and Load Vexter Buffer
	{
		//define vertex with a triangle.
		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		const UINT vbByteSize = sizeof(triangleVertices);

		m_vertexBuffer = nullptr;       //defualt heap
		ComPtr<ID3D12Resource>vertexBufferUpload = nullptr;    //upload heap

		//create defualt buffer
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vbByteSize);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(m_vertexBuffer.GetAddressOf())
		));

		//create upload buffer
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto uploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vbByteSize);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&uploadResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(vertexBufferUpload.GetAddressOf())
		));

		//describe upload resource(blob).
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = triangleVertices;
		subResourceData.RowPitch = vbByteSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		//Release upload to command list.
		//1.change state
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,   //before state
			D3D12_RESOURCE_STATE_COPY_DEST   //after state
		);
		m_commandList->ResourceBarrier(1, &barrier);

		//2.copy
		UpdateSubresources<1>(
			m_commandList.Get(),
			m_vertexBuffer.Get(),     //defualt heap
			vertexBufferUpload.Get(),  //upload heap
			0,
			0,
			1,
			&subResourceData);    //real data

		//rest state
		auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
			m_vertexBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,   //before state
			D3D12_RESOURCE_STATE_COMMON   //after state
		);
		m_commandList->ResourceBarrier(1,&barrier2);

		//Init Vertex Buffer View
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = vbByteSize;
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);

		ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//wait commandList to excute,gpu and cpu sync.
		WaitForPreviousFrame();
	}

		
}


void SampleTriangle::PopulateCommandList()
{
	//reset allocator and list
	ThrowIfFailed(m_commandAllocator[m_frameIndex]->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator[m_frameIndex].Get(), m_pipelineState.Get()));

	//set pipeline state again
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	//swap target render buffer,unlock the buffer to render.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &barrier);

	//get the rtv handle of current buffer to pipeline.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescrptorSize
	);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	//record command.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);   //clear rtv
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  //set primitive topology
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);  //set vertex buffer
	m_commandList->DrawInstanced(3, 1, 0, 0);  //draw triangle

	//swap target render buffer,lock the buffer to present.
	auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_commandList->ResourceBarrier(1, &barrier2);

	//close command list
	ThrowIfFailed(m_commandList->Close());
}

void SampleTriangle::WaitForPreviousFrame()
{
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	if(m_fence->GetCompletedValue() < fence) {
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

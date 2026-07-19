#pragma once

#include "stdafx.h"
#include "RenderAppBase.h"

using namespace DirectX;      //dx math
using namespace Microsoft::WRL;   //comptr orig


class SampleTriangle :public RenderAppBase {
public:
	SampleTriangle(UINT weight, UINT height, std::wstring name);

	// 通过 RenderAppBase 继承
	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

private:
	static const UINT FrameCount = 2;    //swap chain num.

	struct Vertex {    //vertex input layout
		XMFLOAT3 position;    
		XMFLOAT4 color;
	};

	//pipeline object  
	UINT m_rtvDescrptorSize;
	CD3DX12_VIEWPORT m_viewport;    
	CD3DX12_RECT m_scissorRect;   
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];   //swap chain's buffers
	ComPtr<ID3D12CommandAllocator> m_commandAllocator[FrameCount];  //for each swap chain's buffer
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap>m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap>m_cbvHeap;
	ComPtr<ID3D12DescriptorHeap>m_sampleHeap;
	ComPtr<ID3D12PipelineState>m_pipelineState;
	ComPtr <ID3D12GraphicsCommandList> m_commandList;

	//App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	// Synchronization objects.  异步同步相关对象
	UINT m_frameIndex;   //curr swap chain buffer index.
	HANDLE m_fenceEvent;    
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();




};
#include "FrameResource.h"


FrameResource::FrameResource(ID3D12Device* device, UINT passCBElementNums, UINT objectCBElementNums, UINT materialCBElementNums)
{
	//Create command allocator for each frame.
	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	/* todo: constant buffer */
	//当前假定所有常量缓冲区中存储的元素都是ObjectConstants
	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants)) * passCBElementNums;
	UINT objectCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants)) * objectCBElementNums;
	UINT materialCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants)) * materialCBElementNums;

	//Create constant buffer resource for each frame.
	//Constant Buffer real is upload heap;
	{
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(passCBByteSize);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_passCB)));
	}

	{
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(objectCBByteSize);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_objectCB)));
	}

	{
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(materialCBByteSize);
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_materialCB)));
	}

	ThrowIfFailed(m_passCB->Map(0, nullptr, reinterpret_cast<void**>(&mp_passCBMapped)));
	ThrowIfFailed(m_objectCB->Map(0, nullptr, reinterpret_cast<void**>(&mp_objectCBMapped)));
	ThrowIfFailed(m_materialCB->Map(0, nullptr, reinterpret_cast<void**>(&mp_materialCBMapped)));
}

FrameResource::~FrameResource() {
	if (m_passCB) {
		m_passCB->Unmap(0, nullptr);
	}
	if (m_objectCB) {
		m_objectCB->Unmap(0, nullptr);
	}
	if (m_materialCB) {
		m_materialCB->Unmap(0, nullptr);
	}
}
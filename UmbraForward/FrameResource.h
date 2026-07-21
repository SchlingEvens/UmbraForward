#pragma once

#include "stdafx.h"
#include "DXSampleHelper.h"
#include "VertexData.h"
#include "d3dUtil.h"

using Microsoft::WRL::ComPtr;

class FrameResource {
public:
	UINT64 m_fenceValue=0;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	
	//Constant buffer resource for each frame.
	ComPtr<ID3D12Resource> m_passCB;  //pass constant buffer
	ComPtr<ID3D12Resource> m_objectCB;  //object constant buffer
	ComPtr<ID3D12Resource> m_materialCB;  //material constant buffer

	//ptr to mapped data for each frame.
	UINT8* mp_passCBMapped = nullptr;
	UINT8* mp_objectCBMapped = nullptr;
	UINT8* mp_materialCBMapped = nullptr;

public:
	FrameResource(ID3D12Device* device,UINT passCBElementNums,UINT objectCBElementNums,UINT materialCBElementNums);
	~FrameResource();

private:
	FrameResource() = delete;
};
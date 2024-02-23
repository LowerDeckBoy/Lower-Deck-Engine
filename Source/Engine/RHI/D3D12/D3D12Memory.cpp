#include "D3D12Device.hpp"
#include "D3D12Memory.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{
	Ref<D3D12MA::Allocator> D3D12Memory::Allocator = {};

	D3D12Memory::D3D12Memory(D3D12Device* pDevice)
	{
		D3D12MA::ALLOCATOR_DESC desc{};
		desc.pAdapter = pDevice->GetAdapter();
		desc.pDevice = pDevice->GetDevice();
		desc.Flags = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;

		DX_CALL(D3D12MA::CreateAllocator(&desc, &Allocator));	
	}

	D3D12Memory::~D3D12Memory()
	{
		SAFE_RELEASE(Allocator);
	}
	
	void D3D12Memory::Allocate(AllocatedResource& ToAllocate, const D3D12_RESOURCE_DESC& ResourceDesc, AllocType eType, D3D12MA::ALLOCATION_FLAGS AllocationFlags)
	{
		D3D12_HEAP_TYPE heapType{};
		D3D12_RESOURCE_STATES state{};

		switch (eType)
		{
		case lde::RHI::AllocType::eDefault:
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_COMMON;
			break;
		case lde::RHI::AllocType::eUpload:
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		case lde::RHI::AllocType::eCopyDst:
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		}

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = heapType;
		allocDesc.Flags = AllocationFlags;

		DX_CALL(Allocator->CreateResource(&allocDesc, &ResourceDesc, state, nullptr, &ToAllocate.Allocation, IID_PPV_ARGS(&ToAllocate.Resource)));
	}
	
	void D3D12Memory::Allocate(ID3D12Resource** ppResource, D3D12MA::Allocation** ppAllocation, const D3D12_RESOURCE_DESC& HeapDesc, AllocType eType, D3D12MA::ALLOCATION_FLAGS AllocationFlags)
	{
		D3D12_HEAP_TYPE heapType{};
		D3D12_RESOURCE_STATES state{};
		
		switch (eType)
		{
		case lde::RHI::AllocType::eDefault:
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_COMMON;
			break;
		case lde::RHI::AllocType::eUpload:
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		case lde::RHI::AllocType::eCopyDst:
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		}
		
		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = heapType;
		allocDesc.Flags = AllocationFlags;

		DX_CALL(Allocator->CreateResource(&allocDesc, &HeapDesc, state, nullptr, ppAllocation, IID_PPV_ARGS(ppResource)));
	}

	void D3D12Memory::SetFrameIndex(uint32 FrameIndex)
	{
		Allocator->SetCurrentFrameIndex(FrameIndex);
	}
} // namespace lde::RHI

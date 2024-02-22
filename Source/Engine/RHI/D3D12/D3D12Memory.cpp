#include "D3D12Memory.hpp"
#include <AgilitySDK/d3dx12/d3dx12.h>
#include "D3D12Device.hpp"
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
	
		D3D12_HEAP_DESC heapDesc{};
		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
		heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Properties.VisibleNodeMask = 0;
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

		DX_CALL(pDevice->GetDevice()->CreateHeap1(&heapDesc, nullptr, IID_PPV_ARGS(&m_AllocHeap)));
		
	}

	D3D12Memory::~D3D12Memory()
	{
		SAFE_RELEASE(Allocator);
	}

	void D3D12Memory::Allocate(AllocatedResource& ToAllocate, const CD3DX12_RESOURCE_DESC& HeapDesc, AllocType eType, D3D12MA::ALLOCATION_FLAGS AllocationFlags)
	{
		D3D12_HEAP_TYPE heapType{};
		D3D12_RESOURCE_STATES state{};
		if (eType == AllocType::eCopyDst)
		{
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else if (eType == AllocType::eUpload)
		{
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else if (eType == AllocType::eDefault)
		{
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
		}

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = heapType;
		allocDesc.Flags = AllocationFlags;

		DX_CALL(Allocator->CreateResource(&allocDesc, &HeapDesc, state, nullptr, &ToAllocate.Allocation, IID_PPV_ARGS(&ToAllocate.Resource)));
	}

	void D3D12Memory::Allocate(ID3D12Resource* pResource, D3D12MA::Allocation* pAllocation, const CD3DX12_RESOURCE_DESC& HeapDesc, AllocType eType, D3D12MA::ALLOCATION_FLAGS AllocationFlags)
	{
		D3D12_HEAP_TYPE heapType{};
		D3D12_RESOURCE_STATES state{};
		if (eType == AllocType::eCopyDst)
		{
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else if (eType == AllocType::eUpload)
		{
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else if (eType == AllocType::eDefault)
		{
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_COMMON;
		}

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = heapType;
		allocDesc.Flags = AllocationFlags;

		DX_CALL(Allocator->CreateResource(&allocDesc, &HeapDesc, state, nullptr, &pAllocation, IID_PPV_ARGS(&pResource)));
	}

	void D3D12Memory::Allocate(ID3D12Resource** ppResource, D3D12MA::Allocation** ppAllocation, const CD3DX12_RESOURCE_DESC& HeapDesc, AllocType eType, D3D12MA::ALLOCATION_FLAGS AllocationFlags)
	{
		D3D12_HEAP_TYPE heapType{};
		D3D12_RESOURCE_STATES state{};
		if (eType == AllocType::eCopyDst)
		{
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else if (eType == AllocType::eUpload)
		{
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else if (eType == AllocType::eDefault)
		{
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_COMMON;
		}

		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = heapType;
		allocDesc.Flags = AllocationFlags;

		DX_CALL(Allocator->CreateResource(&allocDesc, &HeapDesc, state, nullptr, ppAllocation, IID_PPV_ARGS(ppResource)));
	}

	uint8* D3D12Memory::Map(Ref<ID3D12Resource>& Resource)
	{
		uint8_t* pMapped = nullptr;
		const CD3DX12_RANGE range(0, 0);
		DX_CALL(Resource.Get()->Map(0, &range, reinterpret_cast<void**>(&pMapped)));

		return pMapped;
	}

	void D3D12Memory::CreateReservedResource()
	{

		

	}
}

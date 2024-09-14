#pragma once

/*
	RHI/D3D12/D3D12Memory.hpp

*/

#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>
#include <D3D12MA/D3D12MemAlloc.h>
#include <vector>

namespace lde
{
	class D3D12Device;
	class D3D12CommandList;
	class D3D12Queue;
	class D3D12Fence;

	//https://learn.microsoft.com/en-us/windows/win32/direct3d12/fence-based-resource-management

	enum class AllocType
	{
		eDefault,
		eUpload,
		eCopyDst,
		eGeneric,
	};

	struct AllocatedResource
	{
		Ref<ID3D12Resource>			Resource;
		Ref<D3D12MA::Allocation>	Allocation;
	};

	class D3D12Memory
	{
	public:
		D3D12Memory(D3D12Device* pDevice); /* Creates Allocator */
		~D3D12Memory();	/* Releases Allocator */
		
		static void Allocate(AllocatedResource& ToAllocate,
			const D3D12_RESOURCE_DESC& ResourceDesc,
			AllocType eType,
			D3D12MA::ALLOCATION_FLAGS AllocationFlags = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY);

		static void Allocate(ID3D12Resource** ppResource,
			D3D12MA::Allocation** ppAllocation,
			const D3D12_RESOURCE_DESC& HeapDesc,
			AllocType eType,
			D3D12MA::ALLOCATION_FLAGS AllocationFlags = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY);

		ID3D12Resource* CreateUploadBuffer(D3D12Device* pDevice, uint64 Size, D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE);

		static void SetFrameIndex(uint32 FrameIndex);

		static Ref<D3D12MA::Allocator> Allocator;

	};
} // namespace lde

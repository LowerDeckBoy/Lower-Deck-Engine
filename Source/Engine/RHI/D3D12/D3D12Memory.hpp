#pragma once

/*
	RHI/D3D12/D3D12Memory.hpp

*/

#include <Core/CoreMinimal.hpp>
#include <AgilitySDK/d3d12.h>
#include <AgilitySDK/d3dx12/d3dx12.h>
#include <D3D12MA/D3D12MemAlloc.h>

namespace lde::RHI
{
	class D3D12Device;

	//https://learn.microsoft.com/en-us/windows/win32/direct3d12/fence-based-resource-management

	enum class AllocType
	{
		eDefault,
		eUpload,
		eCopyDst,
	};

	struct AllocatedResource
	{
		Ref<ID3D12Resource>		Resource;
		Ref<D3D12MA::Allocation>	Allocation;
	};

	class D3D12Memory
	{
	public:
		D3D12Memory(D3D12Device* pDevice); /* Creates Allocator */
		~D3D12Memory();	/* Releases Allocator */

		static void Allocate(AllocatedResource& ToAllocate, 
			const CD3DX12_RESOURCE_DESC& HeapDesc,
			AllocType eType,
			D3D12MA::ALLOCATION_FLAGS AllocationFlags = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY);

		static void Allocate(ID3D12Resource* pResource,
			D3D12MA::Allocation* pAllocation,
			const CD3DX12_RESOURCE_DESC& HeapDesc,
			AllocType eType,
			D3D12MA::ALLOCATION_FLAGS AllocationFlags = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY);

		static void Allocate(ID3D12Resource** ppResource,
			D3D12MA::Allocation** ppAllocation,
			const CD3DX12_RESOURCE_DESC& HeapDesc,
			AllocType eType,
			D3D12MA::ALLOCATION_FLAGS AllocationFlags = D3D12MA::ALLOCATION_FLAG_STRATEGY_MIN_MEMORY);
	
		static uint8* Map(Ref<ID3D12Resource>& Resource);
		void Unmap();

		static void CreateReservedResource();

		static Ref<D3D12MA::Allocator> Allocator;

	private:
		Ref<ID3D12Heap1> m_AllocHeap;
		Ref<ID3D12Heap1> m_Heap;
	};

	// TODO: 
	// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device5-createlifetimetracker
	class D3D12LifeTracker
	{
	public:

	private:
		Ref<ID3D12LifetimeTracker> m_Tracker;

	};
	
} // namespace lde::RHI

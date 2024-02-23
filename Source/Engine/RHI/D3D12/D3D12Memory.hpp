#pragma once

/*
	RHI/D3D12/D3D12Memory.hpp

*/

#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>
#include <D3D12MA/D3D12MemAlloc.h>

namespace lde::RHI
{
	class D3D12Device;
	class D3D12CommandList;

	//https://learn.microsoft.com/en-us/windows/win32/direct3d12/fence-based-resource-management

	enum class AllocType
	{
		eDefault,
		eUpload,
		eCopyDst,
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

		static void SetFrameIndex(uint32 FrameIndex);

		static Ref<D3D12MA::Allocator> Allocator;

	private:
		Ref<ID3D12Heap1> m_AllocHeap;
		Ref<ID3D12Heap1> m_Heap;
	};

	class D3D12UploadHeap
	{
	public:
		D3D12UploadHeap(D3D12Device* pDevice);

		ID3D12Resource* GetHeap() { return m_UploadHeap.Get(); }

		void Suballocate();

		void AddBufferCopy(const void* pData, int32 Size, ID3D12Resource* pBufferDst, D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		void AddCopy(D3D12_TEXTURE_COPY_LOCATION Src, D3D12_TEXTURE_COPY_LOCATION Dst);

	private:
		D3D12Device* m_Device = nullptr; /* Parent Device */
		Ref<ID3D12Resource> m_UploadHeap;
		//D3D12CommandList* m_HeapCommandList;

		uint8* m_HeapBegin	= nullptr;
		uint8* m_HeapEnd	= nullptr;
		uint8* m_CurrentPtr = nullptr;

	};

	// TODO: 
	// https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device5-createlifetimetracker
	class D3D12LifeTracker
	{
	public:

	private:
		Ref<ID3D12LifetimeTracker> m_Tracker;

	};

	struct DeletationQueue
	{

	};
	
} // namespace lde::RHI

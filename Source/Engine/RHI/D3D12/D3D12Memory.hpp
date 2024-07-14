#pragma once

/*
	RHI/D3D12/D3D12Memory.hpp

*/

#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>
#pragma warning(push, 0)
#include <D3D12MA/D3D12MemAlloc.h>
#pragma warning(pop)
#include <vector>

namespace lde::RHI
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

	private:
		//Ref<ID3D12Heap1> m_AllocHeap;
		//Ref<ID3D12Heap1> m_Heap;
	};

	

	// TODO: Finish this already
	/*
	class D3D12UploadHeap
	{
	public:
		D3D12UploadHeap(D3D12Device* pDevice, usize HeapSize);

		ID3D12Resource* GetHeap() { return m_UploadHeap.Get(); }

		uint8* Suballocate(usize Size, uint64 Alignment);

		void AddBufferCopy(const void* pData, int32 Size, ID3D12Resource* pBufferDst, D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		void AddCopy(D3D12_TEXTURE_COPY_LOCATION Src, D3D12_TEXTURE_COPY_LOCATION Dst);

		void AddBarrier(ID3D12Resource* pResource);

		// Upload resource and reset queue
		void Flush();

	private:
		D3D12Device*						m_Device = nullptr; // Parent Device
		Ref<ID3D12Resource>					m_UploadHeap;
		std::unique_ptr<D3D12CommandList>	m_HeapCommandList;
		std::unique_ptr<D3D12Queue>			m_HeapQueue;
		std::unique_ptr<D3D12Fence>			m_HeapFence;

		uint8* m_HeapBegin	= nullptr;
		uint8* m_HeapEnd	= nullptr;
		uint8* m_CurrentPtr = nullptr;

		uint8* BeginSuballocate(usize Size, uint64 Alignment);

		struct TextureCopy
		{
			D3D12_TEXTURE_COPY_LOCATION Src, Dst;
			//CD3DX12_TEXTURE_COPY_LOCATION Src, Dst;
		};
		std::vector<TextureCopy> m_TextureCopies;

		struct BufferCopy
		{
			ID3D12Resource* pBufferDst;
			UINT64 offset;
			int size;
			D3D12_RESOURCE_STATES state;
		};
		std::vector<BufferCopy> m_BufferCopies;

		std::vector<D3D12_RESOURCE_BARRIER> m_ToBarrierIntoShaderResource;


	};
	*/
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

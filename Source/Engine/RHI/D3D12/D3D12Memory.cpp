#include "D3D12Device.hpp"
#include "D3D12Memory.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12Queue.hpp"
#include "D3D12Utility.hpp"
#include "Core/Logger.hpp"

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
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		case lde::RHI::AllocType::eUpload:
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		case lde::RHI::AllocType::eCopyDst:
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		case lde::RHI::AllocType::eGeneric:
			heapType = D3D12_HEAP_TYPE_DEFAULT;
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
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

	// =============================================== Upload Heap ===============================================

	D3D12UploadHeap::D3D12UploadHeap(D3D12Device* pDevice, usize HeapSize)
		: m_Device(pDevice)
	{

		m_HeapFence			= std::make_unique<D3D12Fence>(m_Device);
		m_HeapCommandList	= std::make_unique<D3D12CommandList>(m_Device, CommandType::eGraphics, "Upload Heap Command List");
		m_HeapQueue			= std::make_unique<D3D12Queue>(m_Device, CommandType::eGraphics, "Upload Heap Queue");

		auto heapProperties = D3D12Utility::HeapUpload;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Width = static_cast<uint64>(HeapSize);
		desc.Height = 1;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.SampleDesc = { 1, 0 };
		
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_UploadHeap)));
		m_UploadHeap->SetName(L"Upload Heap Resource");

		DX_CALL(m_UploadHeap->Map(0, NULL, reinterpret_cast<void**>(&m_HeapBegin)));

		m_CurrentPtr = m_HeapBegin;
		m_HeapEnd = m_HeapBegin + m_UploadHeap->GetDesc().Width;

	}

	uint8* D3D12UploadHeap::Suballocate(usize Size, uint64 Alignment)
	{
		// wait until we are done flusing the heap
		//flushing.Wait();

		uint8* output = nullptr;
		//std::unique_lock<std::mutex> lock(m_mutex);

		// make sure resource (and its mips) would fit the upload heap, if not please make the upload heap bigger
		usize temp = (usize)(m_HeapEnd - m_HeapBegin);

		LDE_ASSERT(Size < temp);

		m_CurrentPtr = reinterpret_cast<uint8*>(ALIGN(reinterpret_cast<SIZE_T>(m_CurrentPtr), SIZE_T(Alignment)));

		// return nullptr if we ran out of space in the heap
		if (m_CurrentPtr >= m_HeapEnd || m_CurrentPtr + Size >= m_HeapEnd)
		{
			return nullptr;
		}

		output = m_CurrentPtr;
		m_CurrentPtr += Size;
		return output;
	}

	void D3D12UploadHeap::AddBufferCopy(const void* pData, int32 Size, ID3D12Resource* pBufferDst, D3D12_RESOURCE_STATES State)
	{
		//uint8* pixels = BeginSuballocate(Size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		uint8* pixels = BeginSuballocate(Size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
		std::memcpy(pixels, pData, Size);
		//EndSuballocate();

		{
			//std::unique_lock<std::mutex> lock(m_mutex);
			m_BufferCopies.push_back({ pBufferDst, (UINT64)(pixels - m_HeapBegin), Size, State });

			D3D12_RESOURCE_BARRIER RBDesc = {};
			RBDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			RBDesc.Transition.pResource = pBufferDst;
			RBDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			RBDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			RBDesc.Transition.StateAfter = State;
			m_ToBarrierIntoShaderResource.push_back(RBDesc);
		}
	}

	void D3D12UploadHeap::AddBarrier(ID3D12Resource* pResource)
	{
		//std::unique_lock<std::mutex> lock(m_mutex);

		D3D12_RESOURCE_BARRIER RBDesc = {};
		RBDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		RBDesc.Transition.pResource = pResource;
		RBDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		RBDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		RBDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
		//RBDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		m_ToBarrierIntoShaderResource.push_back(RBDesc);
	}

	void D3D12UploadHeap::Flush()
	{        

		for (BufferCopy copy : m_BufferCopies)
		{
			{
				D3D12_RESOURCE_BARRIER desc{};
				desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				desc.Transition.pResource = copy.pBufferDst;
				desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				//desc.Transition.StateBefore = copy.state;
				desc.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
				desc.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
				m_HeapCommandList->Get()->ResourceBarrier(1, &desc);
			}
			m_HeapCommandList->Get()->CopyBufferRegion(copy.pBufferDst, 0, m_UploadHeap.Get(), copy.offset, copy.size);
		}

		m_BufferCopies.clear();

		//apply barriers in one go
		if (m_ToBarrierIntoShaderResource.size() > 0)
		{
			m_HeapCommandList->Get()->ResourceBarrier((UINT)m_ToBarrierIntoShaderResource.size(), m_ToBarrierIntoShaderResource.data());
			m_ToBarrierIntoShaderResource.clear();
		}

		// Close & submit
		DX_CALL(m_HeapCommandList->Get()->Close());
		//m_HeapQueue->Get()->ExecuteCommandLists(1, CommandListCast(&m_pCommandList));
		std::array<ID3D12CommandList*, 1> commandLists{ m_HeapCommandList->Get() };
		m_HeapQueue->Get()->ExecuteCommandLists(static_cast<uint32>(commandLists.size()), commandLists.data());

		// Make sure it's been processed by the GPU
		m_Device->FlushGPU();
		 
		ID3D12CommandQueue* queue = m_HeapQueue->Get();
		ID3D12Fence* pFence;
		DX_CALL(m_Device->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));

		DX_CALL(queue->Signal(pFence, 1));

		HANDLE fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		pFence->SetEventOnCompletion(1, fenceEvent);
		if (fenceEvent)
		{
			::WaitForSingleObject(fenceEvent, INFINITE);
			::CloseHandle(fenceEvent);
		}
		
		SAFE_DELETE(pFence);
		
		// Reset so it can be reused
		m_HeapCommandList->Reset();
		m_CurrentPtr = m_HeapBegin;

		m_HeapFence->Signal(m_HeapQueue.get(), m_HeapFence->GetValue());

		//m_Fence->SetEvent();
		m_HeapFence->Wait();

		m_HeapFence->UpdateValue(m_HeapFence->GetValue());

		//flushing.Dec();
		LOG_INFO("Backend: Upload Heap flushed.");
	}

	uint8* D3D12UploadHeap::BeginSuballocate(usize Size, uint64 Alignment)
	{
		uint8* output = nullptr;
		
		for (;;)
		{
			output = Suballocate(Size, Alignment);
			if (output != nullptr)
			{
				break;
			}

			Flush();
		}

		//allocating.Inc();

		return output;
	}


} // namespace lde::RHI

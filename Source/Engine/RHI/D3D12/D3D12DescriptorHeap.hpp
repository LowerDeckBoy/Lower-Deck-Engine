#pragma once

/*
	RHI/D3D12/D3D12DescriptorHeap.hpp

*/

#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>

#include "D3D12Descriptor.hpp"

namespace lde::RHI
{
	enum class HeapType
	{
		eSRV,
		eRTV,
		eDSV,
	};

	class D3D12Device;

	class D3D12DescriptorHeap
	{
	public:
		D3D12DescriptorHeap(D3D12Device* pDevice, HeapType eType, uint32 MaxCapacity, const LPCWSTR& DebugName = L"");
		~D3D12DescriptorHeap();

		//void Allocate(D3D12Descriptor& OutDescriptor, HeapType eType);

		D3D12Descriptor Allocate(uint32 Count = 1);

		void Allocate(D3D12Descriptor& Descriptor, uint32 Count = 1);

		void Override(D3D12Descriptor& Descriptor, uint32 Count) const;

		ID3D12DescriptorHeap* Get();

		ID3D12DescriptorHeap* const* GetAddressOf();

		uint32 GetDescriptorSize() const;

		uint32 GetIndex(D3D12Descriptor& Descriptor) const;

		inline uint32 GetIndexFromOffset(D3D12Descriptor& Descriptor, uint32 Offset)
		{
			return static_cast<uint32>(
				(Descriptor.GetCpuHandle().ptr + (usize)(Offset * m_DescriptorSize) - m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE CpuStartHandle() const;

		D3D12_GPU_DESCRIPTOR_HANDLE GpuStartHandle() const;

		void Reset();

		HeapType Type{};

	private:
		Ref<ID3D12DescriptorHeap> m_Heap;
		uint32 m_Capacity = 0;
		uint32 m_AllocationsLeft = 0;
		uint32 m_DescriptorSize = 0;

		uint64 m_AvailableCpuPtr = 0;
		uint64 m_AvailableGpuPtr = 0;

		inline bool CanAllocate() const { return (m_AllocationsLeft > 0); }

	};
}

#include "D3D12DescriptorHeap.hpp"
#include "D3D12Device.hpp"
#include "D3D12Utility.hpp"

namespace lde::RHI
{
	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12Device* pDevice, HeapType eType, uint32 MaxCapacity, std::string_view DebugName)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NodeMask = 0;
		desc.NumDescriptors = MaxCapacity;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		switch (eType)
		{
		case HeapType::eSRV:
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			break;
		case HeapType::eRTV:
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case HeapType::eDSV:
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		}

		m_Capacity = MaxCapacity;
		m_AllocationsLeft = m_Capacity;
		m_DescriptorSize = pDevice->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
		Type = eType;

		DX_CALL(pDevice->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap)));

		if (!DebugName.empty())
			m_Heap->SetName(String::ToWide(DebugName).c_str());

		// First handle
		m_AvailableCpuPtr = static_cast<uint64>(m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;
		if (desc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
			m_AvailableGpuPtr = static_cast<uint64>(m_Heap->GetGPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;

	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		Reset();
		SAFE_RELEASE(m_Heap);
	}

	D3D12Descriptor D3D12DescriptorHeap::Allocate(uint32 Count)
	{
		if (!CanAllocate())
		{
			// Warn
		}

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = (D3D12_CPU_DESCRIPTOR_HANDLE)(m_AvailableCpuPtr);
		m_AvailableCpuPtr += ((usize)Count * m_DescriptorSize);

		D3D12Descriptor outputDesc;
		outputDesc.SetCpuHandle(cpuHandle);

		outputDesc.m_Index = GetIndex(outputDesc);

		if (Type == HeapType::eSRV)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = (D3D12_GPU_DESCRIPTOR_HANDLE)(m_AvailableGpuPtr);
			m_AvailableGpuPtr += ((usize)Count * m_DescriptorSize);

			outputDesc.SetGpuHandle(gpuHandle);
		}

		return outputDesc;
	}

	void D3D12DescriptorHeap::Allocate(D3D12Descriptor& Descriptor, uint32 Count)
	{
		if (Descriptor.IsValid())
		{
			Override(Descriptor, Count);
			return;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = (D3D12_CPU_DESCRIPTOR_HANDLE)(m_AvailableCpuPtr);
		m_AvailableCpuPtr += ((size_t)Count * m_DescriptorSize);

		Descriptor.SetCpuHandle(cpuHandle);

		Descriptor.m_Index = GetIndex(Descriptor);

		if (Type == HeapType::eSRV)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = (D3D12_GPU_DESCRIPTOR_HANDLE)(m_AvailableGpuPtr);
			m_AvailableGpuPtr += ((size_t)Count * m_DescriptorSize);

			Descriptor.SetGpuHandle(gpuHandle);
		}
	}

	void D3D12DescriptorHeap::Override(D3D12Descriptor& Descriptor, uint32) const
	{
		uint32 offset = GetIndex(Descriptor);
		D3D12_CPU_DESCRIPTOR_HANDLE cpu = (D3D12_CPU_DESCRIPTOR_HANDLE)(CpuStartHandle().ptr + (offset * m_DescriptorSize));
		Descriptor.SetCpuHandle(cpu);

		if (Type == HeapType::eSRV)
		{
			D3D12_GPU_DESCRIPTOR_HANDLE gpu = (D3D12_GPU_DESCRIPTOR_HANDLE)(GpuStartHandle().ptr + (offset * m_DescriptorSize));
			Descriptor.SetGpuHandle(gpu);
		}
	}

	ID3D12DescriptorHeap* D3D12DescriptorHeap::Get()
	{
		return m_Heap.Get();
	}

	ID3D12DescriptorHeap* const* D3D12DescriptorHeap::GetAddressOf()
	{
		return m_Heap.GetAddressOf();
	}

	uint32 D3D12DescriptorHeap::GetDescriptorSize() const
	{
		return m_DescriptorSize;
	}

	uint32 D3D12DescriptorHeap::GetIndex(D3D12Descriptor& Descriptor) const
	{
		return static_cast<uint32>((Descriptor.GetCpuHandle().ptr - CpuStartHandle().ptr) / m_DescriptorSize);
	}

	uint32 D3D12DescriptorHeap::GetIndexFromOffset(D3D12Descriptor& Descriptor, uint32 Offset) const
	{
		return static_cast<uint32>((Descriptor.GetCpuHandle().ptr + (usize)(Offset * m_DescriptorSize) - CpuStartHandle().ptr) / m_DescriptorSize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::CpuStartHandle() const
	{
		return m_Heap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GpuStartHandle() const
	{
		return m_Heap->GetGPUDescriptorHandleForHeapStart();
	}

	void D3D12DescriptorHeap::Reset()
	{
		m_AllocationsLeft = m_Capacity;

		m_AvailableCpuPtr = static_cast<uint64>(m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;

		if (Type == HeapType::eSRV)
			m_AvailableGpuPtr = static_cast<uint64>(m_Heap->GetGPUDescriptorHandleForHeapStart().ptr) + m_DescriptorSize;
	}
}

#pragma once

/* 
	RHI/D3D12/D3D12Descriptor.hpp
	Bindless descriptor approach.
*/ 

#include <AgilitySDK/d3d12.h>
#include <Core/CoreTypes.hpp>

namespace lde::RHI
{
	class D3D12Descriptor
	{
	public:
		D3D12Descriptor() = default;
		D3D12Descriptor(const D3D12Descriptor& Other)
		{
			this->m_CpuHandle	= Other.m_CpuHandle;
			this->m_GpuHandle	= Other.m_GpuHandle;
			this->m_Index		= Other.m_Index;
		}

		constexpr inline bool IsValid() const
		{
			return m_CpuHandle.ptr != 0;
		}

		inline D3D12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle()
		{
			return m_CpuHandle;
		}

		inline D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const
		{
			return m_GpuHandle;
		}

		inline void SetCpuHandle(D3D12_CPU_DESCRIPTOR_HANDLE Handle)
		{
			m_CpuHandle = Handle;
		}

		inline void SetGpuHandle(D3D12_GPU_DESCRIPTOR_HANDLE Handle)
		{
			m_GpuHandle = Handle;
		}

		inline uint32 Index() const
		{
			return m_Index;
		}

		uint32 m_Index = UINT32_MAX;

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle{};


	};
} // namespace lde::RHI

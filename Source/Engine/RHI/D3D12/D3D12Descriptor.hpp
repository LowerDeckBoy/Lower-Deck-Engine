#pragma once

/* 
	RHI/D3D12/D3D12Descriptor.hpp
	Bindless descriptor approach.
*/ 

#include <AgilitySDK/d3d12.h>
#include <Core/CoreTypes.hpp>

namespace mf::RHI
{
	class D3D12Descriptor
	{
	public:
		constexpr inline bool IsValid() const
		{
			return m_CpuHandle.ptr != 0;
		}

		inline D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const
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
		uint32 m_Index{};

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle{};


	};

}

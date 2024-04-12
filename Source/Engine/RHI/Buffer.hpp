#pragma once

#include "BufferConstants.hpp"
#include <RHI/Types.hpp>

namespace lde::RHI
{
	enum class BufferUsage
	{
		eNone		= 0,
		eVertex		= 1 << 0,
		eIndex		= 1 << 1,
		eConstant	= 1 << 2,
		eStructured = 1 << 3
	};

	struct BufferDesc
	{
		BufferUsage	eType;
		void*		pData;
		uint32		Count;
		usize		Size;
		uint32		Stride;
		// Use only when allocating Descriptor for Buffer
		bool		bBindless = false;
	};

	class Buffer
	{
	public:
		~Buffer() { }

		virtual void Release() = 0;

		virtual void*  GetCpuAddress() const = 0;
		virtual uint64 GetGpuAddress() const = 0;

		virtual void Map(void* pMappedData) = 0;
		virtual void Unmap() = 0;

		virtual uint32 GetSRVIndex() = 0;
		//virtual uint32 GetUAVIndex() = 0;

		BufferDesc GetDesc() const
		{
			return m_Desc;
		}

	protected:
		BufferDesc m_Desc{};

	};

	class ConstantBuffer
	{
	public:
		~ConstantBuffer() {}

		virtual void Update(void* pData) = 0;
		virtual void Release() = 0;

	};

} // namespace lde::RHI

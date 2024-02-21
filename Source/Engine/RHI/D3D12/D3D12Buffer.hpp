#pragma once

/*
	
*/

#include <RHI/Buffer.hpp>

#include "D3D12Descriptor.hpp"
#include "D3D12Memory.hpp"
#include "D3D12Utility.hpp"
#include "RHI/RHICommon.hpp"
#include <Core/CoreMinimal.hpp>
#include <DirectXMath.h>

namespace mf::RHI
{
	class D3D12Context;
	class D3D12Descriptor;
	
	// Single class for Vertex, Index and Structured.
	// Get Index View when binding buffer in order to avoid unnecessary member here
	class D3D12Buffer : public Buffer
	{
	public:
		D3D12Buffer() = default;
		D3D12Buffer(D3D12Context* pGfx, BufferDesc Desc, bool bSRV = false);
		~D3D12Buffer();

		void Create(D3D12Context* pGfx, BufferDesc Desc, bool bSRV = false);

		D3D12Descriptor Descriptor() const;

		void Release() override final;

		void* GetCpuAddress() const override final;
		uint64 GetGpuAddress() const override final;

		void Map(void* pMappedData) override final;
		void Unmap()  override final;

		uint32 GetSRVIndex() override final;
		//virtual uint32 GetUAVIndex()  override final;

		ID3D12Resource* Get() const
		{
			return m_Buffer.Resource.Get();
		}

		
	private:
		AllocatedResource m_Buffer;

		D3D12Descriptor m_Descriptor; /* For Bindless usage */

	};

	/**
	 * @brief Returns Index View from given Buffer
	 * @param Buffer
	 * @return D3D12_INDEX_BUFFER_VIEW() and a warning if BufferUsage is not eIndex
	*/
	extern D3D12_INDEX_BUFFER_VIEW GetIndexView(D3D12Buffer* pBuffer);
	/**
	 * @brief 
	 * @param pBuffer Returns Vertex View from given Buffer
	 * @return D3D12_VERTEX_BUFFER_VIEW() and a warning if BufferUsage is not eVertex
	*/
	extern D3D12_VERTEX_BUFFER_VIEW GetVertexView(D3D12Buffer* pBuffer);

	struct cbPerObject
	{
		DirectX::XMMATRIX WVP   = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
	};

	class D3D12ConstantBuffer
	{
	public:
		D3D12ConstantBuffer() = default;
		D3D12ConstantBuffer(void* pData, usize Size)
		{
			Create(pData, Size);
		}
		~D3D12ConstantBuffer()
		{
			for (uint32 i = 0; i < FRAME_COUNT; i++)
			{
				SAFE_RELEASE(m_Buffers.at(i));
			}
		}

		void Create(void* pData, usize Size);

		// TODO: needs improvement
		void Update(void* pUpdate);

		ID3D12Resource* GetBuffer() { return m_Buffers.at(FRAME_INDEX).Get(); }

		void Release();

		std::array<uint8_t*, FRAME_COUNT> pDataBegin{};

	private:
		std::array<Ref<ID3D12Resource>, FRAME_COUNT> m_Buffers;
		std::array<void*, FRAME_COUNT> m_Data{};
		usize m_Size = 0;

	};
}

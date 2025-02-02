#pragma once

/*
	
*/

#include <RHI/Buffer.hpp>
#include "D3D12Descriptor.hpp"
#include "D3D12Memory.hpp"
#include "RHI/RHICommon.hpp"
#include <Core/CoreMinimal.hpp>
#include <DirectXMath.h>

namespace lde
{
	class D3D12Device;
	class D3D12RHI;
	//class D3D12UploadHeap;
	class D3D12Descriptor;
	class D3D12DescriptorHeap;
	class D3D12Viewport;

	// Single class for Vertex, Index and Structured.
	// Get Index View when binding buffer in order to avoid unnecessary member here
	class D3D12Buffer : public Buffer
	{
	public:
		D3D12Buffer() = default;
		D3D12Buffer(D3D12Device* pDevice, BufferDesc Desc);
		~D3D12Buffer();

		void Create(D3D12Device* pDevice, BufferDesc Desc);

		D3D12Descriptor Descriptor() const;

		void Release() override final;

		[[maybe_unused]]
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

	/**
	 * @brief Creates Resource Desc for Buffer
	 * @param usize 
	 * @return D3D12_RESOURCE_DESC built on given Size
	 */
	extern D3D12_RESOURCE_DESC CreateBufferDesc(usize Size, D3D12_RESOURCE_FLAGS Flag = D3D12_RESOURCE_FLAG_NONE);

	struct cbPerObject
	{
		DirectX::XMMATRIX WVP   = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();
	};

	class D3D12ConstantBuffer : public ConstantBuffer
	{
	public:
		D3D12ConstantBuffer() = default;
		D3D12ConstantBuffer(void* pData, usize Size);
		~D3D12ConstantBuffer();

		void Create(void* pData, usize Size);

		// TODO: needs improvement
		void Update(void* pUpdate) override;

		ID3D12Resource* GetBuffer() { return m_Buffers.at(FRAME_INDEX).Get(); }

		void Release() override;

		std::array<uint8_t*, FRAME_COUNT> pDataBegin{};

	private:
		std::array<Ref<ID3D12Resource>, FRAME_COUNT> m_Buffers;
		std::array<void*, FRAME_COUNT> m_Data{};
		usize m_Size = 0;

	};


	class D3D12DepthBuffer
	{
	public:
		D3D12DepthBuffer() = default;
		D3D12DepthBuffer(D3D12Device* pDevice, D3D12Viewport* pViewport, DXGI_FORMAT Format = DXGI_FORMAT_D32_FLOAT);
		~D3D12DepthBuffer();

		void Create(D3D12Device* pDevice, D3D12Viewport* pViewport);

		void Resize(D3D12Viewport* pViewport);

		ID3D12Resource* Get() { return m_Resource.Get(); }

		inline D3D12Descriptor DSV() const { return m_DSV; }
		inline D3D12Descriptor SRV() const { return m_SRV; }

		void Release();

	private:
		D3D12Device* m_Device = nullptr;
		Ref<ID3D12Resource> m_Resource;
		DXGI_FORMAT m_Format = DXGI_FORMAT_D32_FLOAT;

		D3D12Descriptor m_DSV;
		D3D12Descriptor m_SRV;

	};


} // namespace lde

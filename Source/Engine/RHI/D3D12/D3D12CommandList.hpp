#pragma once

/*
	RHI/D3D12/D3D12CommandList.hpp
	Command Allocators, Lists and Signatures.
*/

#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>
#include <RHI/CommandList.hpp>
#include <RHI/RHICommon.hpp>

namespace lde
{
	enum class CommandType;
	class D3D12Device;
	class D3D12RootSignature;
	
	class D3D12CommandAllocator
	{
	public:
		D3D12CommandAllocator(D3D12Device* pDevice, CommandType eType);
		D3D12CommandAllocator(const D3D12CommandAllocator&) = delete;
		D3D12CommandAllocator(const D3D12CommandAllocator&&) = delete;
		D3D12CommandAllocator& operator=(const D3D12CommandAllocator&) = delete;
		~D3D12CommandAllocator();

		void Reset();
		
		ID3D12CommandAllocator* Get()
		{
			return m_Allocator.Get();
		}

		ID3D12CommandAllocator** GetAddressOf()
		{
			return m_Allocator.GetAddressOf();
		}

		operator ID3D12CommandAllocator*() { return m_Allocator.Get(); }

		CommandType Type;

	private:
		Ref<ID3D12CommandAllocator> m_Allocator;

	};

	class D3D12CommandList : public CommandList
	{
		friend D3D12CommandAllocator;
	public:
		D3D12CommandList(D3D12Device* pDevice, CommandType eType, std::string DebugName = "");
		D3D12CommandList(const D3D12CommandList&) = delete;
		D3D12CommandList(const D3D12CommandList&&) = delete;
		D3D12CommandList& operator=(const D3D12CommandList&) = delete;
		~D3D12CommandList();

		ID3D12GraphicsCommandList8*			Get() const		{ return m_GraphicsCommandList.Get();			}
		ID3D12GraphicsCommandList8* const*	GetAddressOf()	{ return m_GraphicsCommandList.GetAddressOf();	}
		ID3D12CommandAllocator*				GetAllocator()	{ return m_Allocator->Get();						}

		HRESULT Open();
		HRESULT Close();

		//void Reset(D3D12CommandAllocator* pAllocator);

		//void Reset() override final {}
		// Reset Allocator only
		//void ResetAllocator();
		// Reset list only
		void ResetList();

		void DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) override;
		void DrawIndexedInstanced(uint32 Instances, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) override;
		void Draw(uint32 VertexCount) override;
		void DrawIndirect();

		void DispatchRays(const D3D12_DISPATCH_RAYS_DESC& Desc);
		void DispatchMesh(uint32 DispatchX, uint32 DispatchY, uint32 DispatchZ);

		void BindVertexBuffer(Buffer* pBuffer) override final;
		void BindIndexBuffer(Buffer* pBuffer) override final;
		void BindConstantBuffer(uint32 Slot, ConstantBuffer* pBuffer) override;

		void PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset = 0) override;

		void ResourceBarrier(Ref<ID3D12Resource> ppResource, ResourceState Before, ResourceState After);
		// Change state of multiple resources
		void ResourceBarriers(std::span<D3D12_RESOURCE_BARRIER> Barriers);
		void UploadResource(Ref<ID3D12Resource> ppSrc, Ref<ID3D12Resource> ppDst, D3D12_SUBRESOURCE_DATA& Subresource);

	private:
		Ref<ID3D12GraphicsCommandList8> m_GraphicsCommandList;
		D3D12CommandAllocator*			m_Allocator;
		
	};
	
	
	// Helper
	extern D3D12_RESOURCE_STATES StateEnumToType(ResourceState eState);

	// TODO:
	/**/
	class D3D12CommandSignature
	{
	public:
		D3D12CommandSignature();
	
		HRESULT Create(D3D12Device* pDevice, D3D12RootSignature* pRootSignature);

		inline ID3D12CommandSignature* GetSignature() const
		{
			return m_CommandSignature.Get();
		}
		
		void AddConstant(uint32 RootIndex, uint32 Count, uint32 Offset = 0);
		void AddCBV(uint32 Slot);
		void AddSRV(uint32 Slot);
		void AddUAV(uint32 Slot);
		void AddVertexView(uint32 Slot);
		void AddIndexView();

		void AddDraw();
		void AddDrawIndexed();
		void AddDispatch();
		void AddDispatchMesh();
		void AddDispatchRays();

		void AddDrawArgument(uint32 IndexCount, uint32 IndexStart, uint32 VertexStart);

		void Release();

	private:
		Ref<ID3D12CommandSignature> m_CommandSignature;

		std::vector<D3D12_INDIRECT_ARGUMENT_DESC> m_Arguments;

		D3D12_DRAW_INDEXED_ARGUMENTS m_DrawArgs{};
		uint32 m_Stride = 0;
	};
	
} // namespace lde

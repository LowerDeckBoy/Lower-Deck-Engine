#pragma once

/*
	RHI/D3D12/D3D12CommandList.hpp
	Command Allocators, Lists and Signatures.
*/

#include "RHI/RHICommon.hpp"
#include <AgilitySDK/d3d12.h>
#include <Core/CoreMinimal.hpp>
#include <RHI/CommandList.hpp>

namespace mf::RHI
{
	enum class CommandType;
	class D3D12Device;
	class D3D12RootSignature;
	
	class D3D12CommandList : public CommandList
	{
	public:
		D3D12CommandList(D3D12Device* pDevice, CommandType eType, std::string DebugName = "");
		~D3D12CommandList();

		ID3D12GraphicsCommandList6*			Get() const		{ return m_CommandList.Get(); }
		ID3D12GraphicsCommandList6* const*	GetAddressOf()	{ return m_CommandList.GetAddressOf();  }
		ID3D12CommandAllocator*				GetAllocator()	{ return m_Allocators.at(FRAME_INDEX).Get(); }
		[[maybe_unused]]
		std::array<Ref<ID3D12CommandAllocator>, FRAME_COUNT> GetAllocators() { return m_Allocators; }
		
		// Reset both List and Allocator
		void Reset() override final;
		// Reset Allocator only
		void ResetAllocator();
		// Reset list only
		void ResetList();

		void DrawIndexed(uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) override final;
		void DrawIndexedInstanced(uint32 Instances, uint32 IndexCount, uint32 BaseIndex, uint32 BaseVertex) override final;
		void Draw(uint32 VertexCount) override final;
		//void DrawIndirect(uint32 IndexCount, uint32 VertexCount) override;

		void BindVertexBuffer(Buffer* pBuffer) override final;
		void BindIndexBuffer(Buffer* pBuffer) override final;
		void BindConstantBuffer(uint32 Slot, ConstantBuffer* pBuffer) override final;

		void PushConstants(uint32 Slot, uint32 Count, void* pData, uint32 Offset = 0) override final;

	private:
		Ref<ID3D12GraphicsCommandList6> m_CommandList; /* Minimal required for Raytracing */
		std::array<Ref<ID3D12CommandAllocator>, FRAME_COUNT> m_Allocators;

	};

	// TODO:
	class D3D12CommandSignature
	{
	public:
		D3D12CommandSignature(D3D12Device* pDevice, D3D12RootSignature* pRootSignature);

		inline ID3D12CommandSignature* Get() const
		{
			return m_Signature.Get();
		}

	private:
		Ref<ID3D12CommandSignature> m_Signature;
	};
}

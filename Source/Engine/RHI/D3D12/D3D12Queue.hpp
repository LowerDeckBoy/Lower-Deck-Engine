#pragma once

#include <AgilitySDK/d3d12.h>
#include "Core/CoreMinimal.hpp"

namespace lde
{
	enum class CommandType;
	
	class D3D12Device;
	class D3D12Fence;
	class D3D12CommandList;
	
	class D3D12Queue
	{
	public:
		D3D12Queue(D3D12Device* pDevice, CommandType eType, std::string_view DebugName = "");
		~D3D12Queue();

		ID3D12CommandQueue* Get() const
		{
			return m_Queue.Get();
		}

		D3D12Fence& GetFence()
		{
			return m_Fence;
		}

		//void Signal();
		//void Wait();
		//void SignalAndWait();

		void Execute(D3D12CommandList* pCommandList, bool bReset = false);
		void Execute(std::span<D3D12CommandList*> pCommandLists);

		void SignalFence(); 
		void SignalFence(uint64 Value);
		void WaitForComplete();

		void Wait();

		void SignalAndWait();

		void SetFenceValue(uint64 Value)
		{
			m_Fence.Values.at(FRAME_INDEX) = Value;
		}

		CommandType Type;
	private:
		Ref<ID3D12CommandQueue> m_Queue;
		D3D12Fence m_Fence;
		

	};

	/*
	class D3D12Queue
	{
	public:
		D3D12Queue(D3D12Device* pDevice, CommandType eType, std::string_view DebugName = "");
		~D3D12Queue();

		ID3D12CommandQueue* Get() const
		{
			return m_Queue.Get();
		}
		 
		//void Signal();
		//void Wait();
		//void SignalAndWait();

		void Execute(D3D12CommandList* pCommandList);
		void Execute(std::span<D3D12CommandList*> pCommandLists);

		void Signal(D3D12Fence* pFence, uint64 Value);

		void Wait(D3D12Fence* pFence, uint64 Value);

		void SignalAndWait(D3D12Fence* pFence, uint64 Value);
		
		CommandType Type;
	private:
		Ref<ID3D12CommandQueue> m_Queue;

		//D3D12Fence m_Fence;

	};
	*/
} // namespace lde

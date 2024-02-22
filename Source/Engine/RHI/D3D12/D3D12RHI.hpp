#pragma once


#include <RHI/RHI.hpp>
#include <RHI/D3D12/D3D12Device.hpp>
#include <RHI/D3D12/D3D12CommandList.hpp>
#include <RHI/D3D12/D3D12SwapChain.hpp>
//#include <RHI/D3D12/D3D12DescriptorHeap.hpp>

// TODO:
namespace lde::RHI
{
	class D3D12RHI : public RHI
	{
	public:

		void BeginFrame();
		void RecordCommandLists();
		void EndFrame();
		void Update();
		void Render();
		void Present();

	private:
		std::unique_ptr<D3D12Device> m_Device;

	};
} // namespace lde::RHI

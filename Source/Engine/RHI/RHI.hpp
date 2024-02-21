#pragma once

#include <RHI/Device.hpp>
#include <RHI/SwapChain.hpp>
#include <RHI/Buffer.hpp>
#include <RHI/BufferConstants.hpp>
#include <RHI/CommandList.hpp>
#include <RHI/PipelineState.hpp>
#include <RHI/Texture.hpp>

namespace mf::RHI
{
	class RHI
	{
	public:
		~RHI() { }
	
		virtual void BeginFrame() = 0;
		virtual void RecordCommandLists() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;
		virtual void EndFrame() = 0;
		virtual void Present(bool bVSync) = 0;

		// Return RHI specific Device
		virtual Device* GetDevice() = 0;
		// Return RHI specific SwapChain
		virtual SwapChain* GetSwapChain() = 0;

	};
} // namespace mf::RHI

#pragma once

#include "Core/CoreTypes.hpp"
#include <AgilitySDK/d3d12.h>

namespace lde
{
	class D3D12Viewport
	{
	public:
		D3D12Viewport(uint32 Width, uint32 Height)
		{
			Set(Width, Height);
		}

		/// @brief Set both Viewport and Scissor to same dimensions
		/// @param Width New width
		/// @param Height New height
		void Set(uint32 Width, uint32 Height)
		{
			SetViewport(Width, Height);
			SetScissor(Width, Height);
		}

		void SetViewport(uint32 Width, uint32 Height)
		{
			m_Viewport.TopLeftX = 0.0f;
			m_Viewport.TopLeftY = 0.0f;
			m_Viewport.Width	= static_cast<float>(Width);
			m_Viewport.Height	= static_cast<float>(Height);
			m_Viewport.MinDepth = D3D12_MIN_DEPTH;
			m_Viewport.MaxDepth = D3D12_MAX_DEPTH;
		}

		void SetScissor(uint32 Width, uint32 Height)
		{
			m_Scissor.left		= 0L;
			m_Scissor.top		= 0L;
			m_Scissor.right		= static_cast<uint64>(Width);
			m_Scissor.bottom	= static_cast<uint64>(Height);
		}

		D3D12_VIEWPORT GetViewport() const
		{
			return m_Viewport;
		}

		D3D12_RECT GetScissor() const
		{
			return m_Scissor;
		}

	private:
		D3D12_VIEWPORT	m_Viewport{};
		D3D12_RECT		m_Scissor{};

	};
} // namespace lde

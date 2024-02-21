#pragma once

/*
	RHI/D3D12/D3D12Viewport.hpp

*/

#include <Core/CoreTypes.hpp>


namespace mf::RHI
{
	class D3D12Viewport
	{
	public:
		D3D12Viewport(uint32 Width, uint32 Height);

		/// @brief Set both Viewport and Scissor to same dimensions
		/// @param Width New width
		/// @param Height New height
		void Set(uint32 Width, uint32 Height);

		void SetViewport(uint32 Width, uint32 Height);

		void SetScissor(uint32 Width, uint32 Height);

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
}

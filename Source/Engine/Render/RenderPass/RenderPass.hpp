#pragma once

/**
 * @brief Engine/Render/RenderPass/RenderPass.hpp
 * Interface for common RenderPass logic.
 */

namespace lde
{
	/**
	 * @brief Base RenderPass object
	 */
	class RenderPass
	{
	public:

		virtual void Resize(uint32 Width, uint32 Height) = 0;
		virtual void Render() = 0;

	protected:

	};
} // namespace lde

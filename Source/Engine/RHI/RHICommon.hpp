#pragma once

#include "Core/CoreMinimal.hpp"
#include <array>

namespace lde
{
	/// @brief Triple or double buffering
	constexpr uint32 FRAME_COUNT = 2;

	extern uint32 FRAME_INDEX;

	constexpr std::array<float, 4> ClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	
}

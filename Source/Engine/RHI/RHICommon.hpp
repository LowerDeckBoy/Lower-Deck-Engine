#pragma once

/*
	RHI/RHICommon.hpp

*/

#include <Core/CoreMinimal.hpp>
#include <array>


namespace lde
{
	/// @brief Triple buffering
	constexpr uint32 FRAME_COUNT = 2;

	extern uint32 FRAME_INDEX;

	constexpr std::array<float, 4> ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	
}

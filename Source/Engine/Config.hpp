#pragma once

#include "Core/CoreTypes.hpp"

namespace lde
{
	enum SyncInterval
	{
		VSyncOff	= 0,
		VSyncOn		= 1,
		VSyncHalf	= 2
	};

	struct Config
	{
		static Config& Get()
		{
			static Config config;

			return config;
		}

		// Either 2 or 3.
		uint32 NumBackBuffers = 2;

		uint32 SyncInterval = SyncInterval::VSyncOn;

		bool bDrawSky = true;


	};
} // namespace lde

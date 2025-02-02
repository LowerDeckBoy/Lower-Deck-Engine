#pragma once

/*
	
*/

#include <Core/Logger.hpp>
#include <Psapi.h>
#include <chrono>

namespace lde::Utility
{
	class MemoryUsage
	{
	public:
		static float ReadRAM()
		{
			::PROCESS_MEMORY_COUNTERS_EX2 pcmex{};

			if (!::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pcmex, sizeof(::PROCESS_MEMORY_COUNTERS_EX2)))
				return -1.0f;

			return static_cast<float>(pcmex.PrivateWorkingSetSize / (1024.0f * 1024.0f));
		}
	};

	struct LoadTimer
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> StartTime{};
		std::chrono::time_point<std::chrono::high_resolution_clock> EndTime{};
		std::chrono::duration<double> Duration{};
		
		void Start()
		{
			StartTime = std::chrono::high_resolution_clock::now();
		}

		void End(const std::string& Message)
		{
			EndTime = std::chrono::high_resolution_clock::now();
			Duration = EndTime - StartTime;

			if (!Message.empty())
			{
				LOG_INFO(std::format("Loading: {} {}", Message, Duration).c_str());
			}
			else
			{
				LOG_INFO(std::format("Loading time: {}", Duration).c_str());
			}
		}
	};
}

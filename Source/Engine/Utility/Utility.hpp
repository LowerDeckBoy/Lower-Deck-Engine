#pragma once

/*
	
*/

#include <chrono>
#include <Psapi.h>
#include <Core/Logger.hpp>

namespace mf::Utility
{
	class MemoryUsage
	{
	public:
		/// @brief 
		/// @return 
		static float ReadRAM()
		{
			::MEMORYSTATUSEX mem{};
			mem.dwLength = sizeof(::MEMORYSTATUSEX);
			// for convertion to MB and GB
			//constexpr DWORD dwMBFactor = 0x00100000;

			::PROCESS_MEMORY_COUNTERS_EX pcmex{};

			if (!::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pcmex, sizeof(::PROCESS_MEMORY_COUNTERS_EX)))
				return -1.0f;

			if (!::GlobalMemoryStatusEx(&mem))
				return -1.0f;

			return static_cast<float>(pcmex.WorkingSetSize / (1024.0f * 1024.0f));
		}
	};

	struct LoadTimer
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> StartTime{};
		std::chrono::time_point<std::chrono::high_resolution_clock> EndTime{};
		std::chrono::duration<double> Duration{};

	public:
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

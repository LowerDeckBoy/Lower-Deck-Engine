#pragma once

/*

*/

#include "String.hpp"
#include <vector>

#define LOG_INFO(Message)		lde::Log(LogLevel::eInfo, Message)
#define LOG_WARN(Message)		lde::Log(LogLevel::eWarn, Message)
#define LOG_ERROR(Message)		lde::Log(LogLevel::eError, Message)
#define LOG_CRITICAL(Message)	lde::Log(LogLevel::eCritical, Message)
#define LOG_DEBUG(Message)		lde::Log(LogLevel::eDebug, Message)

namespace lde
{
	enum class LogLevel
	{
		eInfo = 0,
		eWarn,
		eError,
		eCritical,
		eDebug,
		COUNT
	};


	extern void Log(LogLevel eLevel, const char* Message);

	// TO REWORK
	class Logger
	{
	public:

		//template<typename... LogArgs>
		//static void Log(LogLevel)

		static std::vector<std::string> Logs;
	};
}



#pragma once

/*

*/

#include <vector>
#include "String.hpp"

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

#define LOG_INFO(Message)		Log(LogLevel::eInfo, Message)
#define LOG_WARN(Message)		Log(LogLevel::eWarn, Message)
#define LOG_ERROR(Message)		Log(LogLevel::eError, Message)
#define LOG_CRITICAL(Message)	Log(LogLevel::eCritical, Message)
#define LOG_DEBUG(Message)		Log(LogLevel::eDebug, Message)

	extern void Log(LogLevel eLevel, const char* Message);

	// TO REWORK
	class Logger
	{
	public:
		static std::vector<std::string> Logs;
	};
}



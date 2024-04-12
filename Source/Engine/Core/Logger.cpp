#include "Logger.hpp"
#include <Core/CoreDefines.hpp>
#include <Core/String.hpp>
#include <Platform/Platform.hpp>

namespace lde
{
	std::vector<std::string> Logger::Logs;

	constexpr const char* PREFIXES[(size_t)LogLevel::COUNT] = { "[Info]", "[Warning]", "[Error]", "[CRITICAL]", "[Debug]" };

	void Log(LogLevel eLevel, const char* Message)
	{
		const auto prefix = PREFIXES[static_cast<size_t>(eLevel)];

		std::string message = std::format("{} {}\n", prefix, Message);
#if PLATFORM_WIN64
		::OutputDebugStringA(message.c_str());
		Logger::Logs.push_back(message);
#else // SDL2 TODO
		std::cout << message;
		Logger::Logs.push_back(message);
#endif

	}
} // namespace lde

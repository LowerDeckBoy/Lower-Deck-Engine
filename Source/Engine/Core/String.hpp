#pragma once

/*
	Core/String.hpp
	String based utility functions.
*/

#include <format>
#include <string>

namespace lde::String
{
	inline std::wstring ToWide(const std::string& Text)
	{
		return std::wstring(Text.begin(), Text.end());
	}

	inline std::wstring ToWide(std::string_view Text)
	{
		return std::wstring(Text.begin(), Text.end());
	}

	inline const char* WCharToChar(wchar_t* Text)
	{
		size_t length = wcslen(Text) + 1;
		char* cstring = new char[length];
		size_t numCharsRead;
		wcstombs_s(&numCharsRead, cstring, length, Text, _TRUNCATE);
		return cstring;
	}
} // namespace lde::String

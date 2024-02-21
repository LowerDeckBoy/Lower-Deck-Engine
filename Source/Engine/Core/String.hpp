#pragma once

/*
	Core/String.hpp
	String based utility functions.
*/

#include <format>
#include <string>

namespace mf::String
{
	inline std::wstring ToWide(std::string Text)
	{
		return std::wstring(Text.begin(), Text.end());
	}

	inline std::wstring ToWide(std::string_view Text)
	{
		return std::wstring(Text.begin(), Text.end());
	}

	inline const char* WCharToChar(wchar_t* Text)
	{
		size_t len = wcslen(Text) + 1;
		char* c_string = new char[len];
		size_t numCharsRead;
		wcstombs_s(&numCharsRead, c_string, len, Text, _TRUNCATE);
		return c_string;
	}
}

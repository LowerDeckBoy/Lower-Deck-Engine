#include "D3D12Utility.hpp"

namespace lde::RHI
{
	void VerifyResult(HRESULT hResult, const char* File, int Line, std::string_view Message)
	{
		if (SUCCEEDED(hResult))
		{
			return;
		}

		char hrError[512]{};
		::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hResult, 0, hrError, (sizeof(hrError) / sizeof(char)), nullptr);

		std::string message = "";
		if (!Message.empty())
		{
			message = std::format("{}\n\n{}\nFile: {}\nLine: {}", Message, hrError, File, Line);
		}
		else
		{
			message = std::format("{}\n\nFile: {}\nLine: {}", hrError, File, Line);
		}

		::MessageBoxA(nullptr, message.c_str(), "D3D12 Error", MB_OK);

		throw std::exception();
	}

	void SetD3D12Name(ID3D12Object* pDxObject, std::string Name)
	{
		pDxObject->SetName(String::ToWide(Name).c_str());
	}
	
} // namespace lde::RHI

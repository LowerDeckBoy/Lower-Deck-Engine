#include "ShaderCompiler.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"
#include <Core/String.hpp>
#include "Utility/FileSystem.hpp"
#include <vector>
#include <Core/Logger.hpp>

#pragma comment(lib, "dxcompiler")


namespace lde
{
	ShaderCompiler* ShaderCompiler::m_Instance = nullptr;

	ShaderCompiler::ShaderCompiler()
	{
		Initialize();
		LOG_INFO("ShaderCompiler initialized.");
	}
	
	ShaderCompiler::~ShaderCompiler()
	{
		Release();
		LOG_INFO("ShaderCompiler released.");
	}
	
	ShaderCompiler& ShaderCompiler::GetInstance()
	{
		if (!m_Instance)
		{
			m_Instance = new ShaderCompiler();
			LOG_DEBUG("ShaderCompiler instance recreated!");
		}
		return *m_Instance;
	}

	void ShaderCompiler::Initialize()
	{
		RHI::DX_CALL(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_DxcCompiler)));
		RHI::DX_CALL(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_DxcUtils)));
		RHI::DX_CALL(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_DxcLibrary)));
		RHI::DX_CALL(m_DxcUtils->CreateDefaultIncludeHandler(&m_DxcIncludeHandler));
		m_Instance = this;
	}

	void ShaderCompiler::Release()
	{
		SAFE_RELEASE(m_DxcLibrary);
		SAFE_RELEASE(m_DxcIncludeHandler);
		SAFE_RELEASE(m_DxcUtils);
		SAFE_RELEASE(m_DxcCompiler);
	}

	Shader ShaderCompiler::Compile(const std::string_view& Filepath, RHI::ShaderStage eType, std::wstring EntryPoint)
	{
		uint32_t codePage = DXC_CP_ACP;
		IDxcBlobEncoding* sourceBlob{};
		RHI::DX_CALL(m_DxcUtils->LoadFile(String::ToWide(Filepath).c_str(), &codePage, &sourceBlob));
	
		auto t = RHI::ShaderEnumToType(eType);
		std::wstring parentPath = String::ToWide(Files::GetParentPath(Filepath));
		std::vector<LPCWSTR> arguments = {
			// Entry point
			L"-E", EntryPoint.c_str(),
			// Target (i.e. vs_6_0)
			L"-T", t.c_str(),
			// Include paths: without them, it can cause issues when trying to do includes inside hlsl
			L"-I Shaders/",
			// HLSL version: 2021 is latest
			L"-HV 2021",
	#if defined (_DEBUG)
			DXC_ARG_DEBUG,
			DXC_ARG_DEBUG_NAME_FOR_SOURCE,
			DXC_ARG_SKIP_OPTIMIZATIONS,
	#else
			DXC_ARG_OPTIMIZATION_LEVEL3
	#endif
		};
	
	
		DxcBuffer buffer{ sourceBlob->GetBufferPointer(), sourceBlob->GetBufferSize(), DXC_CP_ACP };
		IDxcResult* result = nullptr;
		RHI::DX_CALL(m_DxcCompiler.Get()->Compile(&buffer, arguments.data(), static_cast<uint32>(arguments.size()),m_DxcIncludeHandler.Get(), IID_PPV_ARGS(&result)));
	
		IDxcBlobUtf8* errors = nullptr;
		IDxcBlobUtf16* outputName = nullptr;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), &outputName);
		if (errors && errors->GetStringLength() > 0)
		{
			::MessageBoxA(nullptr, (char*)errors->GetBufferPointer(), "Err", MB_OK);
			throw std::runtime_error((char*)errors->GetBufferPointer());
		}
	
		IDxcBlob* blob = nullptr;
		RHI::DX_CALL(result->GetResult(&blob));
	
		return Shader(blob, eType);
	}
} // namespace lde

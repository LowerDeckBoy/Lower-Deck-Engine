#include "Core/FileSystem.hpp"
#include "Core/Logger.hpp"
#include "Core/String.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"
#include "ShaderCompiler.hpp"
#include <AgilitySDK/d3d12.h>
#include <vector>

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
		DX_CALL(DxcCreateInstance(CLSID_DxcCompiler,	IID_PPV_ARGS(&m_DxcCompiler)));
		DX_CALL(DxcCreateInstance(CLSID_DxcUtils,		IID_PPV_ARGS(&m_DxcUtils)));
		DX_CALL(DxcCreateInstance(CLSID_DxcLibrary,		IID_PPV_ARGS(&m_DxcLibrary)));
		DX_CALL(m_DxcUtils->CreateDefaultIncludeHandler(&m_DxcIncludeHandler));
		m_Instance = this;
	}

	void ShaderCompiler::Release()
	{
		SAFE_RELEASE(m_DxcLibrary);
		SAFE_RELEASE(m_DxcIncludeHandler);
		SAFE_RELEASE(m_DxcUtils);
		SAFE_RELEASE(m_DxcCompiler);
	}

	Shader ShaderCompiler::Compile(const std::string_view& Filepath, ShaderStage eType, std::wstring EntryPoint)
	{
		uint32_t codePage = DXC_CP_ACP;
		IDxcBlobEncoding* sourceBlob{};
		DX_CALL(m_DxcUtils->LoadFile(String::ToWide(Filepath).c_str(), &codePage, &sourceBlob));
	
		auto shaderType = ShaderEnumToType(eType);
		std::wstring parentPath = String::ToWide(Files::GetParentPath(Filepath));

		std::vector<LPCWSTR> arguments = {
			// Entry point
			L"-E", EntryPoint.c_str(),
			// Target (i.e. vs_6_0)
			L"-T", shaderType.c_str(),
			// Include paths: without them, it can cause issues when trying to do includes inside hlsl
			L"-I Shaders/",
			L"-I ", parentPath.c_str(),
			// HLSL version: 2021 is latest
			L"-HV 2021",
			DXC_ARG_ALL_RESOURCES_BOUND,
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
		DX_CALL(m_DxcCompiler.Get()->Compile(&buffer, arguments.data(), static_cast<uint32>(arguments.size()), m_DxcIncludeHandler.Get(), IID_PPV_ARGS(&result)));
	
		IDxcBlobUtf8* errors = nullptr;
		IDxcBlobUtf16* outputName = nullptr;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), &outputName);
		if (errors && errors->GetStringLength() > 0)
		{
			std::string errorMessage = std::format("File: {}\n\n{}", Filepath, (char*)errors->GetBufferPointer());
			::MessageBoxA(nullptr, errorMessage.c_str(), "HLSL Error", MB_OK);
			throw std::runtime_error((char*)errors->GetBufferPointer());
		}
	
		IDxcBlob* blob = nullptr;
		DX_CALL(result->GetResult(&blob));
	
		return Shader(blob, eType);
	}
} // namespace lde

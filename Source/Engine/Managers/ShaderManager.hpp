#pragma once

// =====================================================================
// ShaderManager.hpp
//
// =====================================================================

#include <AgilitySDK/d3d12shader.h>
#include <AgilitySDK/d3dx12/d3dx12.h>
#include <dxcapi.h>

#include <Core/CoreMinimal.hpp>
#include <RHI/Types.hpp>
#include <string>

namespace lde
{
	using Microsoft::WRL::ComPtr;
	
	class Shader
	{
	public:
		Shader() {}
		Shader(IDxcBlob* pSource, RHI::ShaderStage eStage) : BinaryData(pSource), Stage(eStage) { }
		~Shader()
		{
			if (BinaryData != nullptr)
			{
				BinaryData->Release();
				BinaryData = nullptr;
			}
		}
		
		inline void* Data() const { return BinaryData->GetBufferPointer(); }
		inline usize Size() const { return BinaryData->GetBufferSize(); }
		inline CD3DX12_SHADER_BYTECODE Bytecode() { return CD3DX12_SHADER_BYTECODE(BinaryData->GetBufferPointer(), BinaryData->GetBufferSize()); }

		IDxcBlob* BinaryData = nullptr;
		RHI::ShaderStage Stage{};
	private:
		std::string m_Filepath;
	
	};

	/// @brief Singleton
	class ShaderManager : public Singleton<ShaderManager>
	{
		friend class Singleton<ShaderManager>;
	public:
		ShaderManager();
		~ShaderManager();
	
		void Initialize();
		void Release();
	
		Shader Compile(const std::string_view& Filepath, RHI::ShaderStage eType, std::wstring EntryPoint = L"main");
	
	private:
		Ref<IDxcCompiler3>		m_DxcCompiler;
		Ref<IDxcUtils>			m_DxcUtils;
		Ref<IDxcIncludeHandler>	m_DxcIncludeHandler;
		Ref<IDxcLibrary>			m_DxcLibrary;
	
		// TODO:
		//void GetWarning();
		//void GetError();
	
	};

} // namespace lde

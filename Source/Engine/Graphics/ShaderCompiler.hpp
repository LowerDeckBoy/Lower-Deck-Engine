#pragma once

/*
	ShaderCompiler.hpp
	Singleton used for compiling and validating HLSL SM6.x shaders.
*/

#include <Core/RefPtr.hpp>
#include <RHI/Types.hpp>
#include <dxcapi.h>

namespace lde
{
	//using Microsoft::WRL::ComPtr;
	
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
		inline D3D12_SHADER_BYTECODE Bytecode() { return D3D12_SHADER_BYTECODE{ BinaryData->GetBufferPointer(), BinaryData->GetBufferSize() }; }

		IDxcBlob* BinaryData = nullptr;
		RHI::ShaderStage Stage{};
	private:
		std::string m_Filepath;
		
	};

	/// @brief Singleton
	class ShaderCompiler
	{
	public:
		ShaderCompiler();
		ShaderCompiler(const ShaderCompiler&) = delete;
		ShaderCompiler(const ShaderCompiler&&) = delete;
		ShaderCompiler operator=(const ShaderCompiler&) = delete;
		~ShaderCompiler();
	
		void Initialize();
		void Release();
	
		Shader Compile(const std::string_view& Filepath, RHI::ShaderStage eType, std::wstring EntryPoint = L"main");
	
		static ShaderCompiler& GetInstance();

	private:
		Ref<IDxcCompiler3>		m_DxcCompiler;
		Ref<IDxcUtils>			m_DxcUtils;
		Ref<IDxcIncludeHandler>	m_DxcIncludeHandler;
		Ref<IDxcLibrary>		m_DxcLibrary;
	
		static ShaderCompiler* m_Instance;

		// TODO:
		//void GetWarning();
		//void GetError();
	
	};

} // namespace lde

#pragma once

/*
	
*/

#include "RHI/D3D12/D3D12Texture.hpp"
#include "ShaderCompiler.hpp"
#include <Core/CoreMinimal.hpp>
#include <unordered_map>
#include <vector>

namespace lde
{
	class D3D12RHI;
	class D3D12Shader;
	class D3D12RootSignature;
	struct D3D12PipelineState;
	
	class TextureManager
	{
		static TextureManager* m_Instance;
	public:
		TextureManager();
		TextureManager(const TextureManager&) = delete;
		TextureManager(const TextureManager&&) = delete;
		TextureManager operator=(const TextureManager&) = delete;
		~TextureManager();
	
		static TextureManager& GetInstance();

		void Initialize(D3D12RHI* pGfx);
		void Release();
		
		/**
		 * @brief Creates texture based on image extension.
		 * @param pGfx 
		 * @param Filepath Filepath Path to image.
		 * @param bGenerateMipMaps Texture object.
		 * @return Index of the newly create Texture. -1 if not created.
		 */
		int32 Create(D3D12RHI* pGfx, std::string_view Filepath, bool bGenerateMipMaps = false);

		//int32 CreateFromDesc(D3D12RHI* pGfx, std::string_view Filepath, D3D12_RESOURCE_DESC& Desc);
		
		// Generate mip chain for 2D texture
		void Generate2D(D3D12Texture* pTexture);

		// Generate mip chain for 3D/TextureCube
		void Generate3D(D3D12Texture* pTexture);

		// Returns mips in chains until 1x1.
		uint16 CountMips(uint32 Width, uint32 Height);

	private:
		/// @brief Loads formats: JPG, JPEG, PNG.
		void Create2D(D3D12RHI* pGfx, std::string_view Filepath, D3D12Texture* pTarget, bool bMipMaps = true);

		void CreateFromHDR(D3D12RHI* pGfx, std::string_view Filepath, D3D12Texture* pTarget);

		/// @brief Loads DDS format textures.
		//void CreateDDS(D3D12RHI* pGfx, std::string_view Filepath, D3D12Texture* pTarget, bool bMipMaps = true);
		
	private:
		D3D12RHI* m_Gfx = nullptr;
		
		void InitializeMipGenerator();
		
		D3D12RootSignature m_RootSignature;
		D3D12PipelineState m_ComputePipeline;

		D3D12RootSignature m_RootSignature3D;
		D3D12PipelineState m_ComputePipeline3D;

		Shader m_ComputeShader;
		Shader m_ComputeShader3D;
		
	};
} // namespace lde

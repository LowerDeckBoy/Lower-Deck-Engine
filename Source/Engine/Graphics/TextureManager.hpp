#pragma once

/*
	
*/

#include "RHI/D3D12/D3D12Texture.hpp"
#include "ShaderCompiler.hpp"
#include <Core/CoreMinimal.hpp>
#include <vector>
#include <unordered_map>

namespace lde
{
	namespace RHI
	{
		class D3D12RHI;
		class D3D12Shader;
		class D3D12RootSignature;
		struct D3D12PipelineState;
	}
	
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

		void Initialize(RHI::D3D12RHI* pGfx);
		void Release();
		
		/**
		 * @brief Creates texture based on image extension.
		 * @param pGfx 
		 * @param Filepath Filepath Path to image.
		 * @param bGenerateMipMaps Texture object.
		 * @return Index of the newly create Texture. -1 if not created.
		 */
		int32 Create(RHI::D3D12RHI* pGfx, std::string_view Filepath, bool bGenerateMipMaps = true);

		int32 CreateFromDesc(RHI::D3D12RHI* pGfx, std::string_view Filepath, D3D12_RESOURCE_DESC& Desc);
		//RHI::D3D12Texture* Create(RHI::D3D12RHI* pGfx, std::string_view Filepath, bool bGenerateMipMaps = true);

		RHI::D3D12Texture* GetTexture(uint32 SRVIndex);

		// Generate mip chain for 2D texture
		void Generate2D(RHI::D3D12Texture* pTexture);
		// Generate mip chain for 3D/TextureCube
		void Generate3D(RHI::D3D12Texture* pTexture);

		std::vector<RHI::D3D12Texture*> m_Textures;
		//std::map<int32, RHI::D3D12Texture*> m_Textures;
		//std::unordered_map<uint32, RHI::D3D12Texture*> m_Textures;

		/// @brief Returns mips in chains until 1x1.
		uint16 CountMips(uint32 Width, uint32 Height);

	private:
		/// @brief Loads formats: JPG, JPEG, PNG.
		void Create2D(RHI::D3D12RHI* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget, bool bMipMaps = true);

		void CreateFromHDR(RHI::D3D12RHI* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget);

		/// @brief Loads DDS format textures.
		//void CreateDDS(RHI::D3D12RHI* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget, bool bMipMaps = true);
		
	private:
		RHI::D3D12RHI* m_Gfx = nullptr;
		
		void InitializeMipGenerator();
		
		RHI::D3D12RootSignature m_RootSignature;
		RHI::D3D12PipelineState m_ComputePipeline;

		RHI::D3D12RootSignature m_RootSignature3D;
		RHI::D3D12PipelineState m_ComputePipeline3D;

		Shader m_ComputeShader;
		Shader m_ComputeShader3D;
		// TODO: 
		// Gather all textures here and only distribute Indices to models
		//std::unordered_map<uint32_t, Texture*> m_Textures;
	};
} // namespace lde
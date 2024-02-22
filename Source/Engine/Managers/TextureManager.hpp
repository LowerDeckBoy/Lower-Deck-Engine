#pragma once

/*
	
*/

#include "RHI/D3D12/D3D12Texture.hpp"
#include <Core/CoreMinimal.hpp>
#include "ShaderCompiler.hpp"
#include <vector>

namespace lde
{
	namespace RHI
	{
		class D3D12Context;
		class D3D12Shader;
		class D3D12RootSignature;
		struct D3D12PipelineState;
	}

	class MipGenerator;

	class TextureManager
	{
		static TextureManager* m_Instance;
	public:
		TextureManager();
		~TextureManager();
	
		static TextureManager& GetInstance();

		void Initialize(RHI::D3D12Context* pGfx);
		void Release();
		
		/**
		 * @brief Creates texture based on image extension.
		 * @param pGfx 
		 * @param Filepath Filepath Path to image.
		 * @param bGenerateMipMaps Texture object.
		 * @return 
		 */
		int32 Create(RHI::D3D12Context* pGfx, std::string_view Filepath, bool bGenerateMipMaps = true);

		std::vector<RHI::D3D12Texture*> m_Textures;

	private:
		/// @brief Loads formats: JPG, JPEG, PNG.
		void Create2D(RHI::D3D12Context* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget, bool bMipMaps = true);

		/// @brief Loads DDS format textures.
		//void CreateDDS(RHI::D3D12Context* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget, bool bMipMaps = true);

		//void CreateFromHDR(const std::string_view& Filepath, RHI::D3D12Texture* pTarget);

		/// @brief Returns mips in chains until 1x1.
		uint16 CountMips(uint32 Width, uint32 Height);

		void CreateSRV(RHI::D3D12Context* pGfx, RHI::D3D12Texture* pTarget, uint16_t Mips, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_SRV_DIMENSION ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D);

		//void CreateUAV(ID3D12Resource** ppResource, RHI::D3D12Descriptor& Descriptor, uint16_t Depth, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);

	private:
		RHI::D3D12Context* m_Gfx = nullptr;

		MipGenerator* m_MipGenerator = nullptr;

		// TODO: 
		// Gather all textures here and only distribute Indices to models
		//std::unordered_map<uint32_t, Texture*> m_Textures;
	};


	class MipGenerator //: public Singleton<MipGenerator>
	{
	public:
		~MipGenerator()
		{
		}
	
		void Release();
	
		static void Initialize(RHI::D3D12Context* pGfx);
	
		inline static RHI::D3D12RootSignature m_RootSignature;
		inline static RHI::D3D12PipelineState m_ComputePipeline;
	
		static void Generate2D(RHI::D3D12Context* pGfx, RHI::D3D12Texture* pTexture);
	
		// TODO:
		//void Generate3D(D3D12Texture& Texture);
	private:
	
		inline static Shader m_ComputeShader;
	
	};

} // namespace lde

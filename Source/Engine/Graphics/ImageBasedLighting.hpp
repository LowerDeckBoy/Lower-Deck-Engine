#pragma once

/**
 * @file ImageBasedLighting.hpp
 * @brief 
 */

#include <Core/CoreMinimal.hpp>
#include <RHI/D3D12/D3D12RHI.hpp>

namespace lde::RHI
{
	class D3D12Device;
}

namespace lde
{	
	class Skybox;
	

	/**
	 * @brief Creates TextureCube from HDRi equirectangular map
	 * and prefilters texture for PBR usage.
	 * Note: it doesn't hold Skybox itself. 
	 */
	class ImageBasedLighting
	{
	public:
		ImageBasedLighting(RHI::D3D12RHI* pRHI, Skybox* pSkybox, std::string_view Filepath);
		~ImageBasedLighting();

		void CreateTextureCube(std::string_view Filepath, Skybox* pSkybox);

		void Create();
		void Release();

	private:
		// Parent
		RHI::D3D12RHI* m_Gfx = nullptr;

		void CreateComputeState();

		void CreateHDRTexture(std::string_view Filepath, Skybox* pSkybox);
		
		Ref<ID3D12RootSignature> m_ComputeRS;
		Ref<ID3D12PipelineState> m_ComputePSO;

		Shader* m_Equirect2CubeCS = nullptr;

	};
} // namespace lde

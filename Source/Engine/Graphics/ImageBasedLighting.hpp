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
		
		RHI::D3D12Texture* SkyboxTexture;
		RHI::D3D12Texture* DiffuseTexture;
		RHI::D3D12Texture* SpecularTexture;

	private:
		// Parent
		RHI::D3D12RHI* m_Gfx = nullptr;

		// Create RootSignature and PSO for executing compute shaders
		void CreateComputeState();
		// Load HDRi texture from file
		void CreateHDRTexture(std::string_view Filepath, Skybox* pSkybox);
		// Transform HDRi equirectangular map into TextureCube
		void CreateTextureCube(Skybox* pSkybox);
		
		void CreateDiffuseTexture(Skybox* pSkybox);
		void CreateSpecularTexture(Skybox* pSkybox);

		struct
		{
			Ref<ID3D12RootSignature> ComputeRS;
			Ref<ID3D12PipelineState> ComputePSO;

			Ref<ID3D12PipelineState> DiffusePSO;
			Ref<ID3D12PipelineState> SpecularPSO;
			Ref<ID3D12PipelineState> BRDFLookUpPSO;
		} m_Pipelines;

		struct
		{
			Shader* Equirect2CubeCS = nullptr;
			Shader* DiffuseIrradianceCS = nullptr;
			Shader* SpecularCS = nullptr;
		} m_Shaders;
		

		

	};
} // namespace lde

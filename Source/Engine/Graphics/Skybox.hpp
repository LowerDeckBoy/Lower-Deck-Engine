#pragma once

#include "RHI/D3D12/D3D12Buffer.hpp"
//#include <RHI/Buffer.hpp>
#include <Scene/Entity.hpp>

namespace lde::RHI
{
	class RHI;
	class Texture;
	class D3D12Device;
	class D3D12Texture;
}

namespace lde
{
	class SceneCamera;
	
	// https://github.com/TheRealMJP/DXRPathTracer/blob/master/SampleFramework12/v1.02/Graphics/Skybox.cpp
	class Skybox : public Entity
	{
	public:
		Skybox() {}
		~Skybox();

		void Create(RHI::RHI* pRHI, World* pWorld, std::string_view Filepath);

		/**
		 * @brief Draw given texture as a Skybox
		 * @param TextureID Index of a Texture to draw
		 * @param ViewProjection From Camera
		 */
		void Draw(int32 TextureID, SceneCamera* pCamera);

		int32& GetTextureIndex() { return m_TextureIndex; }
		
		//struct
		//{
		//	RHI::D3D12Texture* Equirectangular	= nullptr;
		//	RHI::D3D12Texture* TextureCube		= nullptr;
		//	RHI::D3D12Texture* Irradiance		= nullptr;
		//	RHI::D3D12Texture* Specular			= nullptr;
		//	RHI::D3D12Texture* BRDFLookUp		= nullptr;
		//} Textures;
		
		// Equirectangular; pre-transform
		RHI::D3D12Texture* Texture = nullptr;
		// Actual TextureCube resource
		RHI::D3D12Texture* TextureCube = nullptr;
		//  
		RHI::D3D12Texture* DiffuseTexture = nullptr;
		//
		RHI::D3D12Texture* SpecularTexture = nullptr;
		// BDRF Look-up Texture
		RHI::D3D12Texture* BRDFTexture = nullptr;

	private:
		RHI::D3D12Device* m_Device = nullptr;
		
		BufferHandle m_IndexBuffer = (uint32)INVALID_HANDLE;
		// Per object
		BufferHandle m_ConstBuffer = (uint32)INVALID_HANDLE;
		RHI::cbPerObject m_cbPerObject{};
		
		int32 m_TextureIndex = -1;

		// PSO here

	};
} // namespace lde

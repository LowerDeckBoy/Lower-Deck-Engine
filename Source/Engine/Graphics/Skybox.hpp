#pragma once

#include "RHI/D3D12/D3D12Buffer.hpp"
#include <Scene/Entity.hpp>

namespace lde
{
	class RHI;
	class Texture;
	class D3D12Device;
	class D3D12Texture;
	class SceneCamera;
	
	// TODO: https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/master/src/DX12/shaders/SkyDome.hlsl
	class Skybox : public Entity
	{
	public:
		Skybox() = default;
		~Skybox();

		void Create(RHI* pRHI, World* pWorld, std::string_view Filepath);

		/**
		 * @brief Draw given texture as a Skybox
		 * @param TextureID Index of a Texture to draw
		 * @param ViewProjection From Camera
		 */
		void Draw(int32 TextureID, SceneCamera* pCamera);

		int32& GetTextureIndex() { return m_TextureIndex; }
		
		// Equirectangular; pre-transformed
		D3D12Texture* Texture = nullptr;
		// Actual TextureCube resource
		D3D12Texture* TextureCube = nullptr;
		//  
		D3D12Texture* DiffuseTexture = nullptr;
		//
		D3D12Texture* SpecularTexture = nullptr;
		// BDRF Look-up Texture
		D3D12Texture* BRDFTexture = nullptr;

		int32 BRDF_LUT;

	private:
		D3D12Device* m_Device = nullptr;
		
		BufferHandle m_IndexBuffer = (uint32)INVALID_HANDLE;
		// Per object
		BufferHandle m_ConstBuffer = (uint32)INVALID_HANDLE;
		cbPerObject m_cbPerObject{};
		
		int32 m_TextureIndex = -1;

	};
} // namespace lde

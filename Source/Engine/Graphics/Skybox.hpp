#pragma once

#include <RHI/Buffer.hpp>
#include <Scene/Entity.hpp>

namespace lde::RHI
{
	class RHI;
	class Texture;
	class D3D12Device;
}

namespace lde
{
	class SceneCamera;

	// https://devblogs.microsoft.com/directx/directx-innovation-on-display-at-gdc-2024/
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

	private:
		RHI::D3D12Device* m_Device = nullptr;

		RHI::Buffer* m_VertexBuffer = nullptr;
		RHI::Buffer* m_IndexBuffer = nullptr;
		// Per object
		RHI::ConstantBuffer* m_ConstBuffer = nullptr;
		RHI::cbPerObject m_cbPerObject{};
		
		int32 m_TextureIndex = -1;

		

		// PSO here

	};
} // namespace lde

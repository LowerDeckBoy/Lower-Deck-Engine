#include <RHI/D3D12/D3D12Texture.hpp>
#include <Scene/Scene.hpp>
#include "LightPass.hpp"
#include "GBufferPass.hpp"
#include <RHI/D3D12/D3D12RHI.hpp>
#include <Graphics/Skybox.hpp>

namespace lde
{
	LightPass::LightPass(D3D12RHI* pGfx)
	{	
		m_Gfx = pGfx;
		Create(pGfx);
	}

	LightPass::~LightPass()
	{	
		m_Texture.reset();
	}

	void LightPass::Render(SceneCamera* pCamera, GBufferPass* pGBuffer, Skybox* pSkybox, Scene* pScene)
	{
		// Send Shader data
		m_SceneData.CameraPosition	= pCamera->GetPosition();
		m_SceneData.View			= pCamera->GetView();
		m_SceneData.InversedView	= XMMatrixTranspose(pCamera->GetInvView());
		m_SceneData.Projection		= pCamera->GetProjection();
		m_SceneData.InversedProjection = XMMatrixTranspose(pCamera->GetInvProjection());

		auto* sceneConstBuffer = m_Gfx->Device->ConstantBuffers.at(m_SceneConstBuffer);
		sceneConstBuffer->Update(&m_SceneData);
		m_Gfx->BindConstantBuffer(sceneConstBuffer, 0);
		
		if (!pScene->PointLights.empty())
		{
			for (usize i = 0; i < 4; ++i)
			{
				m_LightsData.Light[i] = pScene->PointLights.at(i)->GetComponent<PointLightComponent>();
			}
		}
			
		if (!pScene->DirectionalLights.empty())
		{
			m_LightsData.Directional = pScene->DirectionalLights.at(0)->GetComponent<DirectionalLightComponent>();
		}
		
		auto* lightingConstBuffer = m_Gfx->Device->ConstantBuffers.at(m_SceneLighting);
		lightingConstBuffer->Update(&m_LightsData);
		m_Gfx->BindConstantBuffer(lightingConstBuffer, 1);

		auto indices = pGBuffer->GetTextureIndices();
		m_Gfx->Device->GetGfxCommandList()->PushConstants(2, 7, indices.data());
		
		struct
		{
			uint32 irradiance;
			uint32 specular;
			uint32 brdf;
		} iblIndices { 
			.irradiance = pSkybox->DiffuseTexture->SRV.Index(), 
			.specular = pSkybox->SpecularTexture->SRV.Index(), 
			.brdf = (uint32)pSkybox->BRDF_LUT
		};
		m_Gfx->Device->GetGfxCommandList()->PushConstants(3, 3, &iblIndices);

		// Note:
		// Actually there's no need for Index Buffer
		// Can draw screen output from shader only
		// m_Gfx->BindIndexBuffer(m_IndexBuffer);
		// m_Gfx->DrawIndexed(6, 0, 0);

		m_Gfx->Draw(4);

	}

	void LightPass::Resize(uint32 Width, uint32 Height)
	{
		m_Texture->OnResize(Width, Height);
	}

	void LightPass::Create(D3D12RHI* pGfx)
	{
		m_Texture = std::make_unique<D3D12RenderTexture>();
		m_Texture->Initialize(pGfx, DXGI_FORMAT_R32G32B32A32_FLOAT, "Light Pass Render Texture");

		// Getting indices inside of a shader.
		//std::array<uint32, 6> indices = { 0, 1, 2, 2, 3, 0 };
		//m_IndexBuffer = pGfx->GetDevice()->CreateBuffer(BufferDesc(BufferUsage::eIndex, indices.data(), 6, 24, 4, false));

		m_SceneConstBuffer = pGfx->GetDevice()->CreateConstantBuffer(&m_SceneData, sizeof(m_SceneData));

		m_SceneLighting = pGfx->GetDevice()->CreateConstantBuffer(&m_LightsData, sizeof(m_LightsData));

	}
} // namespace lde

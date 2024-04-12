#include <RHI/D3D12/D3D12Texture.hpp>
#include <Scene/SceneCamera.hpp>
#include "LightPass.hpp"
#include "GBufferPass.hpp"
#include <RHI/D3D12/D3D12RHI.hpp>
#include <Graphics/Skybox.hpp>

namespace lde
{
	LightPass::LightPass(RHI::D3D12RHI* pGfx)
	{	
		m_Gfx = pGfx;
		Create(pGfx);
	}

	LightPass::~LightPass()
	{	
		m_SceneConstBuffer->Release(); 
		m_SceneLighting->Release();
		m_Texture.reset();
	}

	void LightPass::Render(SceneCamera* pCamera, GBufferPass* pGBuffer, Skybox* pSkybox)
	{
		m_Gfx->TransitResource(m_Texture->Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto& depthHandle = m_Gfx->SceneDepth->DSV().GetCpuHandle();
		auto& rtvHandle = m_Texture->GetRTV().GetCpuHandle();

		m_Gfx->ClearRenderTarget(rtvHandle);
		m_Gfx->SetRenderTarget(rtvHandle, &depthHandle);

		// Send Shader data

		m_SceneData.CameraPosition = pCamera->GetPosition();
		m_SceneData.View = pCamera->GetView();
		m_SceneData.InversedView = XMMatrixTranspose(XMMatrixInverse(nullptr, pCamera->GetView()));
		m_SceneData.Projection = pCamera->GetProjection();
		m_SceneData.InversedProjection = XMMatrixTranspose(XMMatrixInverse(nullptr, pCamera->GetProjection()));
		m_SceneData.zNear = pCamera->GetZNear();
		m_SceneData.zFar = pCamera->GetZFar();
		m_SceneData.Width  = static_cast<uint32>(m_Gfx->SceneViewport->GetViewport().Width);
		m_SceneData.Height = static_cast<uint32>(m_Gfx->SceneViewport->GetViewport().Height);

		m_SceneConstBuffer->Update(&m_SceneData);
		m_Gfx->BindConstantBuffer(m_SceneConstBuffer, 0);

		// TEST
		m_LightsData.Light[0] = m_PointLights.at(0);
		m_LightsData.Light[1] = m_PointLights.at(1);
		m_LightsData.Light[2] = m_PointLights.at(2);
		m_LightsData.Light[3] = m_PointLights.at(3);

		m_LightsData.Directional = { .Direction = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), .Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f) };

		m_SceneLighting->Update(&m_LightsData);
		m_Gfx->BindConstantBuffer(m_SceneLighting, 1);

		auto indices = pGBuffer->GetTextureIndices();
		((RHI::D3D12RHI*)m_Gfx)->Device->GetGfxCommandList()->PushConstants(2, 7, indices.data());

		struct
		{
			uint32 skybox;
			uint32 irradiance;
			uint32 specular;
			uint32 brdf;
		} iblIndices { 
			.skybox = pSkybox->TextureCube->SRV.Index(), 
			.irradiance = pSkybox->DiffuseTexture->SRV.Index(), 
			.specular = pSkybox->SpecularTexture->SRV.Index(), 
			.brdf = pSkybox->BRDFTexture->SRV.Index() 
		};
		((RHI::D3D12RHI*)m_Gfx)->Device->GetGfxCommandList()->PushConstants(3, 4, &iblIndices);
		

		// Note:
		// Actually there's no need for Index Buffer
		// Can draw screen output from shader only
		// m_Gfx->BindIndexBuffer(m_IndexBuffer);
		// m_Gfx->DrawIndexed(6, 0, 0);

		m_Gfx->Draw(4);
		
		m_Gfx->TransitResource(m_Texture->Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	void LightPass::Resize(uint32 Width, uint32 Height)
	{
		m_Texture->OnResize(Width, Height);
	}

	void LightPass::Create(RHI::D3D12RHI* pGfx)
	{
		m_Texture = std::make_unique<RHI::D3D12RenderTexture>();
		m_Texture->Initialize(pGfx, DXGI_FORMAT_R32G32B32A32_FLOAT, "Light Pass Render Texture");

		// Getting indices inside of a shader.
		//std::array<uint32, 6> indices = { 0, 1, 2, 2, 3, 0 };
		//m_IndexBuffer = pGfx->GetDevice()->CreateBuffer(RHI::BufferDesc(RHI::BufferUsage::eIndex, indices.data(), 6, 24, 4, false));

		m_SceneConstBuffer = pGfx->GetDevice()->CreateConstantBuffer(&m_SceneData, sizeof(m_SceneData));

		//m_SceneLighting = pGfx->GetDevice()->CreateConstantBuffer(&m_DirLight, sizeof(m_DirLight));
		

		PointLightComponent light;
		light.Position = XMFLOAT4(-8.0f, 1.0f, 0.5f, 1.0f);
		light.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light.Range = 25.0f;
		m_PointLights.push_back(light);

		light.Position = XMFLOAT4(-5.0f, 1.0f, 0.5f, 1.0f);
		m_PointLights.push_back(light);

		light.Position = XMFLOAT4(0.0f, 1.0f, 0.5f, 1.0f);
		m_PointLights.push_back(light);

		light.Position = XMFLOAT4(5.0f, 1.0f, 0.5f, 1.0f);
		m_PointLights.push_back(light);

		m_SceneLighting = pGfx->GetDevice()->CreateConstantBuffer(&m_LightsData, sizeof(m_LightsData));


	}
} // namespace lde

#pragma once

/*

*/

#include <Core/CoreMinimal.hpp>

#include "RHI/D3D12/D3D12RHI.hpp"

#include "Scene/Model/Model.hpp"
#include "Scene/SceneLighting.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include "Graphics/AssetManager.hpp"
#include "Graphics/TextureManager.hpp"

#include <map>

#include "Graphics/Skybox.hpp"
#include "Graphics/ImageBasedLighting.hpp"
// RenderPasses
#include "RenderPass/GBufferPass.hpp"
#include "RenderPass/LightPass.hpp"
#include "RenderPass/SkyPass.hpp"

namespace lde
{
	class Scene;

	/// @brief For displaying desired image
	enum class RenderOutput : uint8
	{
		//eShaded = 0,
		eShaded = 0,
		eDepth,
		eBaseColor,
		eTexCoords,
		eNormal,
		eMetalRoughness,
		eEmissive,
		eWorldPosition,
		eSkybox,
		eAmbientOcclusion,
		eRaytracing,
		COUNT
	};

	class Renderer
	{
	public:
		Renderer(RHI::D3D12RHI* pGfx);
		Renderer(RHI::D3D12RHI* pGfx, Scene* pScene);
		~Renderer();

		void Initialize();
		void OnResize(uint32 Width, uint32 Height);
		void Release();

		void BeginFrame();
		void EndFrame();

		void RecordCommands();

		void Update();
		void Render();
		void Present();
		
		void SetScene(Scene* pScene);

		static bool bVSync;

	private:
		RHI::D3D12RHI* m_Gfx = nullptr;
		//RHI::RHI* m_Gfx = nullptr;
		Scene* m_ActiveScene = nullptr;

		std::unique_ptr<ShaderCompiler>	m_ShaderCompiler;
		std::unique_ptr<TextureManager> m_TextureManager;
		std::unique_ptr<AssetManager>	m_AssetManager;
		
		// PSOs
		RHI::D3D12RootSignature m_GBufferRS;
		RHI::D3D12PipelineState m_GBufferPSO;

		RHI::D3D12RootSignature m_LightRS;
		RHI::D3D12PipelineState m_LightPSO;

		RHI::D3D12RootSignature m_SkyboxRS;
		RHI::D3D12PipelineState m_SkyboxPSO;

		void BuildRootSignatures();
		void BuildPipelines();

	public:
		//D3D12RenderTexture m_BaseRTV;

		std::unique_ptr<Skybox> m_Skybox;
		std::unique_ptr<ImageBasedLighting> m_IBL;

		SceneLighting* m_SceneLighting = nullptr;

		GBufferPass*	m_GBufferPass = nullptr;
		LightPass*		m_LightPass = nullptr;
		SkyPass*		m_SkyPass = nullptr;

		/// @brief Gets Render Target for Editor image output
		/// @return pointer to Render Target at SelectedRenderTarget
		uint64 GetRenderTarget();

		RenderOutput SelectedRenderTarget = RenderOutput::eShaded;
		//RenderOutput SelectedRenderTarget = RenderOutput::eSkybox;

	};
} // namespace lde

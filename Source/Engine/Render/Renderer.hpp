#pragma once

/*

*/

#include <Core/CoreMinimal.hpp>

#include "RHI/D3D12/D3D12RHI.hpp"

#include "Graphics/AssetManager.hpp"
#include "Graphics/ImageBasedLighting.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include "Graphics/Skybox.hpp"
#include "Graphics/TextureManager.hpp"
#include "Scene/Model/Model.hpp"
#include <map>
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
		Renderer(D3D12RHI* pGfx);
		Renderer(D3D12RHI* pGfx, Scene* pScene);
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
		D3D12RHI* m_Gfx = nullptr;
		
		Scene* m_ActiveScene = nullptr;

		D3D12RenderTexture SceneImage;

		std::unique_ptr<ShaderCompiler>	m_ShaderCompiler;
		std::unique_ptr<TextureManager> m_TextureManager;
		std::unique_ptr<AssetManager>	m_AssetManager;
		
		// PSOs
		D3D12RootSignature m_GBufferRS;
		D3D12PipelineState m_GBufferPSO;

		D3D12RootSignature m_LightRS;
		D3D12PipelineState m_LightPSO;

		D3D12RootSignature m_SkyboxRS;
		D3D12PipelineState m_SkyboxPSO;

		D3D12RootSignature m_MeshletRS;
		D3D12PipelineState m_MeshletPSO;

		void BuildRootSignatures();
		void BuildPipelines();

		BufferHandle   m_SceneConstBuffer = 0;
		SceneData m_SceneData{};

	public:
		std::unique_ptr<Skybox> m_Skybox;
		std::unique_ptr<ImageBasedLighting> m_IBL;

		GBufferPass*	m_GBufferPass = nullptr;
		LightPass*		m_LightPass = nullptr;
		SkyPass*		m_SkyPass = nullptr;

		/// @brief Gets Render Target for Editor image output
		/// @return pointer to Render Target at SelectedRenderTarget
		uint64 GetRenderTarget();

		RenderOutput SelectedRenderTarget = RenderOutput::eShaded;

	};
} // namespace lde

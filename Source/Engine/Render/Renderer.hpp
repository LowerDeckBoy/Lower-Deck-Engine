#pragma once

/*

*/

#include <Core/CoreMinimal.hpp>

#include "RHI/D3D12/D3D12Context.hpp"

#include "Scene/Model/Model.hpp"
#include "Managers/ShaderManager.hpp"
#include "Managers/AssetManager.hpp"
#include "Managers/TextureManager.hpp"

#include <map>

#include "RenderPass/GBufferPass.hpp"

namespace mf
{
	class Scene;

	/// @brief For displaying desired image
	enum class RenderOutput : uint8
	{
		//eShaded = 0,
		eDepth = 0,
		eBaseColor,
		eTexCoords,
		eNormal,
		eMetalRoughness,
		eEmissive,
		eWorldPosition,
		eShadows,
		eAmbientOcclusion,
		eRaytracing,
		eScene, // Final scene texture
		COUNT
	};

	class Renderer : public Singleton<Renderer>
	{
		friend class Singleton<Renderer>;
	public:
		Renderer(RHI::D3D12Context* pGfx);
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
		
		void SetHeaps();

		void SetScene(Scene* pScene);

		static bool bVSync;

	private:
		RHI::D3D12Context* m_Gfx = nullptr;
		//RHI::RHI* m_Gfx = nullptr;
		Scene* m_ActiveScene = nullptr;

		std::unique_ptr<ShaderManager>	m_ShaderManager;
		std::unique_ptr<TextureManager> m_TextureManager;
		std::unique_ptr<AssetManager>	m_AssetManager;
		std::unique_ptr<MipGenerator>	m_MipGenerator;

		RHI::D3D12RootSignature m_BaseRootSignature;
		RHI::D3D12PipelineState m_BasePipeline;

		void BuildRootSignatures();
		void BuildPipelines();

	public:
		//D3D12RenderTexture m_BaseRTV;

		GBufferPass* m_GBufferPass = nullptr;

		/// @brief Gets Render Target for Editor image output
		/// @return pointer to Render Target at SelectedRenderTarget
		uint64 GetRenderTarget();

		RenderOutput SelectedRenderTarget = RenderOutput::eNormal;

	};
} // namespace mf

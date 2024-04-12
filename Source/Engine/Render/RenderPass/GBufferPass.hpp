#pragma once

/*
	
*/

#include "RHI/D3D12/D3D12Texture.hpp"
#include "RHI/D3D12/D3D12RootSignature.hpp"
#include "RHI/D3D12/D3D12PipelineState.hpp"
#include <Core/CoreTypes.hpp>
#include <map>

namespace lde::RHI
{
	class D3D12RHI;
	class D3D12RenderTexture;
	class D3D12RootSignature;
	struct D3D12PipelineState;
}

namespace lde
{
	class Scene;

	/*
		
	*/
	struct PassContent
	{
		RHI::D3D12RenderTexture GeometryDepth;
		RHI::D3D12RenderTexture BaseColor;
		RHI::D3D12RenderTexture TexCoords;
		RHI::D3D12RenderTexture Normal;
		RHI::D3D12RenderTexture MetalRoughness;
		RHI::D3D12RenderTexture WorldPosition;
	};

	enum class GBuffers : uint8
	{
		eDepth,
		eBaseColor,
		eTexCoords,
		eNormal,
		eMetalRoughness,
		eEmissive,
		eWorldPosition,
		COUNT
	};

	class GBufferPass
	{
	public:
		/// @brief Initializes Root Signature and Pipeline State
		/// @param pGfx 
		GBufferPass(RHI::D3D12RHI* pGfx);

		void Render(Scene* pScene);
		void OnResize(uint32 Width, uint32 Height);

		std::map<GBuffers, RHI::D3D12RenderTexture> GetRenderTargets() const
		{
			return m_RenderTargets;
		}

		void Release();
		
		std::array<int, 7> GetTextureIndices();

	private:
		PassContent m_GBuffer{};
		//D3D12_RENDER_PASS_RENDER_TARGET_DESC

		RHI::D3D12RHI* m_Gfx = nullptr;
		RHI::D3D12RootSignature m_RootSignature;
		RHI::D3D12PipelineState m_PipelineState;

		std::map<GBuffers, RHI::D3D12RenderTexture> m_RenderTargets =
		{
			{ GBuffers::eDepth,				RHI::D3D12RenderTexture() },
			{ GBuffers::eBaseColor,			RHI::D3D12RenderTexture() },
			{ GBuffers::eTexCoords,			RHI::D3D12RenderTexture() },
			{ GBuffers::eNormal,			RHI::D3D12RenderTexture() },
			{ GBuffers::eMetalRoughness,	RHI::D3D12RenderTexture() },
			{ GBuffers::eEmissive,			RHI::D3D12RenderTexture() },
			{ GBuffers::eWorldPosition,		RHI::D3D12RenderTexture() },
		};

	};

}

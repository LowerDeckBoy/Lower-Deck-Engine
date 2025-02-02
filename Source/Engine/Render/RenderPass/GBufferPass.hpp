#pragma once

/*
	
*/

#include "RHI/D3D12/D3D12Texture.hpp"
#include "RHI/D3D12/D3D12RootSignature.hpp"
#include "RHI/D3D12/D3D12PipelineState.hpp"
#include <Core/CoreTypes.hpp>
#include <map>

namespace lde
{
	class D3D12RHI;
	class D3D12RenderTexture;
	class D3D12RootSignature;
	class D3D12CommandSignature;
	struct D3D12PipelineState;
	class Scene;
	
	struct PassContent
	{
		D3D12RenderTexture GeometryDepth;
		D3D12RenderTexture BaseColor;
		D3D12RenderTexture TexCoords;
		D3D12RenderTexture Normal;
		D3D12RenderTexture MetalRoughness;
		D3D12RenderTexture WorldPosition;
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
		GBufferPass(D3D12RHI* pGfx);
		~GBufferPass();

		void Render(Scene* pScene);
		void Resize(uint32 Width, uint32 Height);

		std::map<GBuffers, D3D12RenderTexture> GetRenderTargets() const
		{
			return m_RenderTargets;
		}

		void Release();
		
		std::array<int, 7> GetTextureIndices();

	private:
		PassContent m_GBuffer{};

		D3D12RHI* m_Gfx = nullptr;
		D3D12RootSignature m_RootSignature;
		D3D12PipelineState m_PipelineState;

		std::map<GBuffers, D3D12RenderTexture> m_RenderTargets =
		{
			{ GBuffers::eDepth,				D3D12RenderTexture() },
			{ GBuffers::eBaseColor,			D3D12RenderTexture() },
			{ GBuffers::eTexCoords,			D3D12RenderTexture() },
			{ GBuffers::eNormal,			D3D12RenderTexture() },
			{ GBuffers::eMetalRoughness,	D3D12RenderTexture() },
			{ GBuffers::eEmissive,			D3D12RenderTexture() },
			{ GBuffers::eWorldPosition,		D3D12RenderTexture() },
		};
	};
} // namespace lde

#include "RHI/D3D12/D3D12RHI.hpp"
#include "TextureManager.hpp"
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#pragma warning(pop)
#include <Utility/FileSystem.hpp>
#include <Core/Logger.hpp>
#include <Core/String.hpp>

namespace lde
{
	TextureManager* TextureManager::m_Instance = nullptr;
	
	TextureManager::TextureManager()
	{
		m_Instance = this;
		LOG_DEBUG("TextureManager initialized.");
	}

	TextureManager::~TextureManager()
	{
		LOG_DEBUG("TextureManager released.");
	}

	TextureManager& TextureManager::GetInstance()
	{
		if (!m_Instance)
		{
			m_Instance = new TextureManager();
			LOG_DEBUG("TextureManager instance recreated!");
		}

		return *m_Instance;
	}

	void TextureManager::Initialize(RHI::D3D12RHI* pGfx)
	{
		m_Gfx = pGfx;
		InitializeMipGenerator();
	}

	void TextureManager::Release()
	{
		
	}

	int32 TextureManager::Create(RHI::D3D12RHI* pGfx, std::string_view Filepath, bool bGenerateMipMaps)
	{
		if (Filepath.empty())
		{	
			LOG_WARN("Filepath not given or couldn't be found. Could not create a Texture object.");
			return -1;
		}
	
		RHI::D3D12Texture* newTexture = new RHI::D3D12Texture();
		
		const auto extension = Files::ImageExtToEnum(Filepath);
	
		switch (extension)
		{
		case Files::ImageExtension::eJPG:	[[fallthrough]];
		case Files::ImageExtension::eJPEG:	[[fallthrough]];
		case Files::ImageExtension::ePNG:	[[fallthrough]];
		case Files::ImageExtension::eTGA:	[[fallthrough]];
		case Files::ImageExtension::eBMP:
		{
			Create2D(pGfx, Filepath, newTexture, bGenerateMipMaps);
			break;
		}
		case Files::ImageExtension::eHDR:
		{
			CreateFromHDR(pGfx, Filepath, newTexture);
			break;
		}
		default:
			LOG_ERROR("Invalid texture extension!");
			return -1;
		}
		
		m_Gfx->Device->CreateTexture(newTexture);
		return static_cast<uint32>(newTexture->SRV.Index());
	}

	void TextureManager::Create2D(RHI::D3D12RHI* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget, bool bMipMaps)
	{
		int32 width, height, channels = 3;
		void* pixels = stbi_load(Filepath.data(), &width, &height, &channels, STBI_rgb_alpha);
		if (!pixels)
		{
			::MessageBoxA(nullptr, stbi_failure_reason(), "Error", MB_OK);
			throw std::runtime_error("");
		}

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width				= static_cast<uint64>(width);
		desc.Height				= static_cast<uint32>(height);
		desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.DepthOrArraySize	= 1;
		desc.SampleDesc			= { 1, 0 };
		desc.Alignment			= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags				= (bMipMaps) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
	
		const uint16 mipCount = (bMipMaps) ? CountMips(static_cast<uint32_t>(desc.Width), desc.Height) : 1;
		desc.MipLevels = mipCount;
	
		RHI::DX_CALL(pGfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(pTarget->Texture.ReleaseAndGetAddressOf())
		));
		
		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData		= pixels;
		subresource.RowPitch	= static_cast<LONG_PTR>(desc.Width * 4u);
		subresource.SlicePitch	= static_cast<LONG_PTR>(subresource.RowPitch * desc.Height);

		D3D12_RESOURCE_DESC uploadBufferDesc{};
		uploadBufferDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
		uploadBufferDesc.Width				= subresource.SlicePitch;
		uploadBufferDesc.Height				= 1;
		uploadBufferDesc.MipLevels			= 1;
		uploadBufferDesc.DepthOrArraySize	= 1;
		uploadBufferDesc.SampleDesc			= { 1, 0 };
		uploadBufferDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		Ref<ID3D12Resource> uploadResource;
		RHI::DX_CALL(pGfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapUpload,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadResource.ReleaseAndGetAddressOf())
		));
		uploadResource->SetName(L"Texture Upload Resource");
		
		pGfx->UploadResource(pTarget->Texture, uploadResource, subresource);
		pGfx->TransitResource(pTarget->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	
		pTarget->MipLevels	= mipCount;
		pTarget->Width		= static_cast<uint32>(desc.Width);
		pTarget->Height		= desc.Height;
	
		pGfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);

		pGfx->Device->CreateSRV(pTarget->Texture.Get(), pTarget->SRV, mipCount, 1);

		stbi_image_free(pixels);
		SAFE_RELEASE(uploadResource);

		if (bMipMaps)
		{
			Generate2D(pTarget);
		}
	}

	void TextureManager::CreateFromHDR(RHI::D3D12RHI* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget)
	{	
		int32 width, height, channels = 0;
		
		stbi_ldr_to_hdr_scale(1.0f);
		stbi_ldr_to_hdr_gamma(2.2f);
		float* pixels = stbi_loadf(Filepath.data(), &width, &height, &channels, STBI_rgb_alpha);
		if (!pixels)
		{
			::MessageBoxA(nullptr, stbi_failure_reason(), "Error", MB_OK);
			throw std::runtime_error("");
		}

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width				= static_cast<uint64>(width);
		desc.Height				= static_cast<uint32>(height);
		desc.Format				= DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.MipLevels			= 1;
		desc.DepthOrArraySize	= 1;
		desc.SampleDesc			= { 1, 0 };
		desc.Alignment			= D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;

		RHI::DX_CALL(pGfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(pTarget->Texture.ReleaseAndGetAddressOf())
		));

		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData		= pixels;
		subresource.RowPitch	= static_cast<LONG_PTR>(desc.Width * 16u);
		subresource.SlicePitch	= static_cast<LONG_PTR>(subresource.RowPitch * desc.Height);

		D3D12_RESOURCE_DESC uploadBufferDesc{};
		uploadBufferDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
		uploadBufferDesc.Width				= subresource.SlicePitch;
		uploadBufferDesc.Height				= 1;
		uploadBufferDesc.MipLevels			= 1;
		uploadBufferDesc.DepthOrArraySize	= 1;
		uploadBufferDesc.SampleDesc			= { 1, 0 }; 
		uploadBufferDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		Ref<ID3D12Resource> uploadResource;
		RHI::DX_CALL(pGfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapUpload,
			D3D12_HEAP_FLAG_NONE,
			&uploadBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadResource.ReleaseAndGetAddressOf())
		));
		uploadResource->SetName(L"Texture Upload Resource");

		pGfx->UploadResource(pTarget->Texture, uploadResource, subresource);
		pGfx->TransitResource(pTarget->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		pGfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);

		pGfx->Device->CreateSRV(pTarget->Texture.Get(), pTarget->SRV, 1, 1);

		stbi_image_free(pixels);
		SAFE_RELEASE(uploadResource);
		
	}

	uint16 TextureManager::CountMips(uint32 Width, uint32 Height)
	{
		uint16 count = 1;
		while (Width > 1 || Height > 1)
		{
			Width = Width >> 1;
			Height = Height >> 1;
			count++;
		}
	
		return count;
	}

	void TextureManager::InitializeMipGenerator()
	{
		// 2D
		{
			m_RootSignature.AddConstants(8, 0);
			m_RootSignature.AddStaticSampler(0, 0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_NEVER);
			m_RootSignature.Build(m_Gfx->Device.get(), RHI::PipelineType::eCompute, "MipMap2D Root Signature");

			m_ComputeShader = ShaderCompiler::GetInstance().Compile("Shaders/Compute/MipMap2D.hlsl", RHI::ShaderStage::eCompute, L"CSmain");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = m_RootSignature.Get();
			psoDesc.CS = m_ComputeShader.Bytecode();
			psoDesc.NodeMask = 0;
			psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_ComputePipeline.PipelineState)));
			m_ComputePipeline.Type = RHI::PipelineType::eCompute;
			m_ComputePipeline.PipelineState->SetName(L"MipMap2D Compute Pipeline State");
		}
		
		// 3D/TextureCube
		{
			m_RootSignature3D.AddConstants(8, 0);
			m_RootSignature3D.AddStaticSampler(0, 0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
			m_RootSignature3D.Build(m_Gfx->Device.get(), RHI::PipelineType::eCompute, "MipMap3D Root Signature");

			m_ComputeShader3D = ShaderCompiler::GetInstance().Compile("Shaders/Compute/MipMap3D.hlsl", RHI::ShaderStage::eCompute, L"CSmain");

			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = m_RootSignature3D.Get();
			psoDesc.CS = m_ComputeShader3D.Bytecode();
			psoDesc.NodeMask = 0;
			psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_ComputePipeline3D.PipelineState)));
			m_ComputePipeline3D.Type = RHI::PipelineType::eCompute;
			m_ComputePipeline3D.PipelineState->SetName(L"MipMap3D Compute Pipeline State");
		}

	}

	void TextureManager::Generate2D(RHI::D3D12Texture* pTexture)
	{
		if (!pTexture->Texture.Get())
			return;

		auto srcResourceDesc = pTexture->Texture->GetDesc();

		if (pTexture->MipLevels == 1 || pTexture->Width == 1)
			return;

		// check if array == 6
		if (srcResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
			srcResourceDesc.DepthOrArraySize != 1 ||
			srcResourceDesc.SampleDesc.Count > 1)
		{
			return;
		}

		Ref<ID3D12Resource> uavResource = pTexture->Texture;

		struct cbMipData
		{
			uint32 SrcMipIndex;
			uint32 DestMipIndex;
			uint32 SrcMipLevel;
			uint32 NumMips;
			uint32 SrcDimension;
			uint32 IsSRGB;
			DirectX::XMFLOAT2 TexelSize;
		} mipGenCB{};

		((RHI::D3D12Device*)m_Gfx->GetDevice())->Allocate(RHI::HeapType::eSRV, pTexture->UAV, pTexture->MipLevels);

		m_Gfx->SetPipeline(&m_ComputePipeline);
		m_Gfx->SetRootSignature(&m_RootSignature);

		mipGenCB.IsSRGB = srcResourceDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? 1 : 0;
		
		auto heap = ((RHI::D3D12Device*)m_Gfx->GetDevice())->GetSRVHeap();
		for (uint32 srcMip = 0; srcMip < (uint32)(pTexture->MipLevels - 1); ++srcMip)
		{
			uint64 srcWidth = srcResourceDesc.Width >> srcMip;
			uint32 srcHeight = srcResourceDesc.Height >> srcMip;
			uint32 destWidth = static_cast<uint32>(srcWidth >> 1);
			uint32 destHeight = srcHeight >> 1;

			mipGenCB.SrcDimension = static_cast<uint32>(((usize)srcHeight & 1) << 1 | (srcWidth & 1));
			unsigned long mipCount = 0;

			_BitScanForward(&mipCount, (destWidth == 1 ? destHeight : destWidth) | (destHeight == 1 ? destWidth : destHeight));

			mipCount = std::min<unsigned long>(1, mipCount - 1);
			mipCount = (static_cast<uint32>(srcMip) + mipCount) >= pTexture->MipLevels ? pTexture->MipLevels - srcMip - 1 : mipCount;

			destWidth = std::max(1u, destWidth);
			destHeight = std::max(1u, destHeight);

			mipGenCB.SrcMipLevel = srcMip;
			mipGenCB.NumMips = mipCount;
			mipGenCB.TexelSize.x = 1.0f / (float)destWidth;
			mipGenCB.TexelSize.y = 1.0f / (float)destHeight;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = srcResourceDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = pTexture->MipLevels;

			m_Gfx->Device->GetDevice()->CreateShaderResourceView(uavResource.Get(), &srvDesc, pTexture->SRV.GetCpuHandle());

			for (uint32_t mip = 0; mip < mipCount; ++mip)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
				uavDesc.Format = srcResourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

				m_Gfx->Device->GetDevice()->CreateUnorderedAccessView(
					uavResource.Get(), nullptr, &uavDesc,
					{ pTexture->UAV.GetCpuHandle().ptr + ((srcMip + mip) * m_Gfx->Device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) });
			}

			m_Gfx->Device->GetGfxCommandList()->Get()->SetDescriptorHeaps(1, heap->GetAddressOf());

			mipGenCB.SrcMipIndex = heap->GetIndexFromOffset(pTexture->SRV, 0);
			mipGenCB.DestMipIndex = heap->GetIndexFromOffset(pTexture->UAV, srcMip);

			m_Gfx->Device->GetGfxCommandList()->Get()->SetComputeRoot32BitConstants(0, 8, &mipGenCB, 0);

			m_Gfx->Device->GetGfxCommandList()->Get()->Dispatch(((destWidth + 8u - 1) / 8u), (destHeight + 8u - 1) / 8u, 1);

			//CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(uavResource.Get());
			D3D12_RESOURCE_BARRIER uavBarrier{};
			uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			uavBarrier.UAV.pResource = uavResource.Get();
			m_Gfx->Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &uavBarrier);
		}

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		// Reset UAV; they are not necessary after mip creation for now
		pTexture->UAV = {};
		
	}
		
	void TextureManager::Generate3D(RHI::D3D12Texture* pTexture)
	{
		if (!pTexture->Texture.Get())
		{
			LOG_WARN("Texture was not created. Skipping generating 3D mip chain.");
			return;
		}
	
		if (pTexture->MipLevels == 1)
		{
			LOG_DEBUG("3D Texture Mip levels equal to 1. Skipping mip chain generation.");
			return;
		}
	
		const auto srcDesc = pTexture->Texture->GetDesc();
		ID3D12Resource* uavResource = pTexture->Texture.Get();

		// Ensure that the Heap is set before mipmapping
		m_Gfx->Device->GetGfxCommandList()->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
		
		RHI::D3D12Descriptor srvResourceDesc;
		m_Gfx->Device->Allocate(RHI::HeapType::eSRV, srvResourceDesc, 1);

		struct
		{
			uint32 SrcMipIndex;  // SRV index
			uint32 DestMipIndex; // UAV index
			uint32 SrcMipLevel;
			uint32 NumMips;
			uint32 IsSRGB;
			DirectX::XMFLOAT3 TexelSize;
		} cbMipData{};

		for (uint32 arraySlice = 0; arraySlice < 6; ++arraySlice)
		{
			m_Gfx->Device->GetGfxCommandList()->Get()->SetComputeRootSignature(m_RootSignature3D.Get());
			m_Gfx->Device->GetGfxCommandList()->Get()->SetPipelineState(m_ComputePipeline3D.Get());

			for (uint32 srcMip = 0; srcMip < srcDesc.MipLevels; ++srcMip)
			{
				const float baseMipWidth	= static_cast<float>(pTexture->Width >> srcMip);
				const float baseMipHeight   = static_cast<float>(pTexture->Height >> srcMip);
				const float baseMipDepth	= static_cast<float>(srcDesc.DepthOrArraySize >> srcMip);
				
				cbMipData.SrcMipLevel = srcMip;
				cbMipData.NumMips = 1;
				cbMipData.IsSRGB = false;
				cbMipData.TexelSize = DirectX::XMFLOAT3(2.0f / baseMipWidth, 2.0f / baseMipHeight, 2.0f / baseMipDepth);
				cbMipData.SrcMipIndex = srvResourceDesc.Index();
	
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = srcDesc.Format;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.MipLevels = pTexture->MipLevels;
				srvDesc.Texture2DArray.PlaneSlice = 0;
				srvDesc.Texture2DArray.FirstArraySlice = arraySlice;
				srvDesc.Texture2DArray.ArraySize = 1;

				m_Gfx->Device->GetDevice()->CreateShaderResourceView(uavResource, &srvDesc, srvResourceDesc.GetCpuHandle());

				m_Gfx->Device->Allocate(RHI::HeapType::eSRV, pTexture->UAV, srcDesc.MipLevels); //  * 6

				for (uint32_t mip = 0; mip < pTexture->MipLevels; ++mip)
				{
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
					uavDesc.Format = srcDesc.Format;
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDesc.Texture2DArray.MipSlice = mip;
					uavDesc.Texture2DArray.PlaneSlice = 0;
					uavDesc.Texture2DArray.FirstArraySlice = arraySlice;
					uavDesc.Texture2DArray.ArraySize = 1;

					m_Gfx->Device->GetDevice()->CreateUnorderedAccessView(
						uavResource, nullptr, &uavDesc,
						{ pTexture->UAV.GetCpuHandle().ptr + ((srcMip + mip + arraySlice) * m_Gfx->Device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))});
				}

				cbMipData.SrcMipIndex = m_Gfx->Device->GetSRVHeap()->GetIndexFromOffset(srvResourceDesc, 0);
				cbMipData.DestMipIndex = m_Gfx->Device->GetSRVHeap()->GetIndexFromOffset(pTexture->UAV, srcMip + arraySlice + 6);

				m_Gfx->Device->GetGfxCommandList()->Get()->SetComputeRoot32BitConstants(0, 8, &cbMipData, 0);

				const uint32 dispatchX = std::max((uint32)std::ceil(baseMipWidth  / (2.0f * 8.0f)), 1u);
				const uint32 dispatchY = std::max((uint32)std::ceil(baseMipHeight / (2.0f * 8.0f)), 1u);
				const uint32 dispatchZ = std::max((uint32)std::ceil(baseMipDepth  / 2.0f), 1u);
				m_Gfx->Device->GetGfxCommandList()->Get()->Dispatch(dispatchX, dispatchY, dispatchZ);

				//CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(uavResource);
				D3D12_RESOURCE_BARRIER uavBarrier{};
				uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				uavBarrier.UAV.pResource = uavResource;
				m_Gfx->Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &uavBarrier);
			}

			// Execute after every ArraySlice step
			m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		}
	}

} // namespace lde

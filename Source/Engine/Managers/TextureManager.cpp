#include "RHI/D3D12/D3D12Context.hpp"
#include "RHI/D3D12/D3D12PipelineState.hpp"
#include "RHI/D3D12/D3D12RootSignature.hpp"
#include "TextureManager.hpp"
#include "Core/String.hpp"

#include <DirectXMath.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "Utility/FileSystem.hpp"

namespace mf
{

	class MipGenerator;

	int32 TextureManager::Create(RHI::D3D12Context* pGfx, std::string_view Filepath, bool bGenerateMipMaps)
	{
		RHI::D3D12Texture* newTexture = new RHI::D3D12Texture();
		
		const auto extension = files::ImageExtToEnum(Filepath);
	
		switch (extension)
		{
		case files::ImageExtension::eJPG:
		case files::ImageExtension::eJPEG:
		case files::ImageExtension::ePNG:
		case files::ImageExtension::eTGA:
		case files::ImageExtension::eBMP:
		{
			Create2D(pGfx, Filepath, newTexture, bGenerateMipMaps);
			break;
		}
		//case files::ImageExtension::eDDS:
		//{
		//	//CreateDDS(pGfx, Filepath, newTexture, true);
		//	break;
		//}
		//case files::ImageExtension::eHDR:
		//{
		//	// TODO: HDR
		//	break;
		//}
		default:
			throw std::runtime_error("");
		}
	
		m_Textures.emplace_back(newTexture);
		return static_cast<uint32>(newTexture->SRV.Index());
	}

	void TextureManager::Create2D(RHI::D3D12Context* pGfx, std::string_view Filepath, RHI::D3D12Texture* pTarget, bool bMipMaps)
	{
		auto path = String::ToWide(Filepath);
	
		int32 width, height, channels;
		void* pixels = stbi_load(Filepath.data(), &width, &height, &channels, STBI_rgb_alpha);
		if (!pixels)
		{
			::MessageBoxA(nullptr, stbi_failure_reason(), "Error", MB_OK);
			throw std::runtime_error("");
		}
		
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = static_cast<uint64_t>(width);
		desc.Height = static_cast<uint32_t>(height);
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc = { 1, 0 };
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = (bMipMaps) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
	
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
	
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTarget->Texture.Get(), 0, 1);
		const auto uploadBuffer = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	
		Ref<ID3D12Resource> uploadResource;
		RHI::DX_CALL(pGfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapUpload,
			D3D12_HEAP_FLAG_NONE,
			&uploadBuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadResource.ReleaseAndGetAddressOf())
		));
		uploadResource->SetName(L"Texture Upload Resource");

		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData = pixels;
		subresource.RowPitch = static_cast<LONG_PTR>(desc.Width * 4u);
		subresource.SlicePitch = static_cast<LONG_PTR>(subresource.RowPitch * desc.Height);
	
		pGfx->UploadResource(pTarget->Texture, uploadResource, subresource);
		pGfx->TransitResource(pTarget->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	
		pTarget->MipLevels	= mipCount;
		pTarget->Width		= static_cast<uint32>(desc.Width);
		pTarget->Height		= desc.Height;
	
		pGfx->ExecuteCommandList(pGfx->GraphicsCommandList, pGfx->Device->GetGfxQueue(), true);

		CreateSRV(pGfx, pTarget, mipCount, desc.Format);

		stbi_image_free(pixels);
		SAFE_RELEASE(uploadResource);

		if (bMipMaps)
		{
			MipGenerator::GetInstance().Generate2D(pGfx, pTarget);
		}
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

	void TextureManager::CreateSRV(RHI::D3D12Context* pGfx, RHI::D3D12Texture* pTarget, uint16_t Mips, DXGI_FORMAT Format, D3D12_SRV_DIMENSION ViewDimension)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = Format;
		if (ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = Mips;
			srvDesc.Texture2D.MostDetailedMip = 0;
		}
		else if (ViewDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = Mips;
			srvDesc.TextureCube.MostDetailedMip = 0;
		}
	
		pGfx->Heap->Allocate(pTarget->SRV, 1);
		pGfx->Device->GetDevice()->CreateShaderResourceView(pTarget->Texture.Get(), &srvDesc, pTarget->SRV.GetCpuHandle());
	}

	void MipGenerator::Release()
	{
		SAFE_RELEASE(m_ComputePipeline.PipelineState);
		m_RootSignature.Release();
	}

	void MipGenerator::Initialize(RHI::D3D12Context* pGfx)
	{
		std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(1);
		rootParameters[0].InitAsConstants(8, 0);
	
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
	
		D3D12_STATIC_SAMPLER_DESC staticSampler{};
		staticSampler.Filter = D3D12_FILTER_ANISOTROPIC;
		staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
		staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		staticSampler.MinLOD = 0.0f;
		staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
		staticSampler.MipLODBias = 0;
		staticSampler.ShaderRegister = 0;
		staticSampler.RegisterSpace = 0;
		staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = {};
		versionedRootSignatureDesc.Init_1_1(static_cast<uint32>(rootParameters.size()), rootParameters.data(), 1, &staticSampler, rootSignatureFlags);
	
		Ref<ID3DBlob> serializedRootSig;
		Ref<ID3DBlob> errorBlob;
	
		RHI::DX_CALL(D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, &serializedRootSig, &errorBlob));
	
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers{ staticSampler };
		m_RootSignature.Create(pGfx->Device.get(), rootParameters, samplers, rootSignatureFlags, L"MipMap Root Signature");
		m_RootSignature.Type = RHI::PipelineType::eCompute;
		m_RootSignature.GetRootSignature()->SetName(L"MipMap Root Signature");
		//m_ComputeShader = ShaderManager::GetInstance().Compile("../Moonfolk/Shaders/MipMap2D.hlsl", RHI::ShaderStage::eCompute, L"CSmain");
		m_ComputeShader = ShaderManager::GetInstance().Compile("Shaders/MipMap2D.hlsl", RHI::ShaderStage::eCompute, L"CSmain");
	
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = m_RootSignature.GetRootSignature();
		psoDesc.CS = m_ComputeShader.Bytecode();
		psoDesc.NodeMask = 0;
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	
		//m_ComputePipeline = new RHI::D3D12PipelineState();
		RHI::DX_CALL(pGfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_ComputePipeline.PipelineState)));
		m_ComputePipeline.Type = RHI::PipelineType::eCompute;
		m_ComputePipeline.PipelineState->SetName(L"MipMap Compute Pipeline State");
	}

	void MipGenerator::Generate2D(RHI::D3D12Context* pGfx, RHI::D3D12Texture* pTexture)
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

		//pGfx->Heap->Allocate(pTexture->SRV, 1);
		pGfx->Heap->Allocate(pTexture->UAV, pTexture->MipLevels);

		pGfx->SetPipeline(&m_ComputePipeline);
		pGfx->SetRootSignature(&m_RootSignature);

		mipGenCB.IsSRGB = srcResourceDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? 1 : 0;

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
			
			pGfx->Device->GetDevice()->CreateShaderResourceView(uavResource.Get(), &srvDesc, pTexture->SRV.GetCpuHandle());

			for (uint32_t mip = 0; mip < mipCount; ++mip)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
				uavDesc.Format = srcResourceDesc.Format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

				pGfx->Device->GetDevice()->CreateUnorderedAccessView(
					uavResource.Get(), nullptr, &uavDesc, 
					{ pTexture->UAV.GetCpuHandle().ptr + ((srcMip + mip) * pGfx->Device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)) });
			}

			pGfx->GraphicsCommandList->Get()->SetDescriptorHeaps(1, pGfx->MipMapHeap->GetAddressOf());

			mipGenCB.SrcMipIndex  = pGfx->MipMapHeap->GetIndexFromOffset(pTexture->SRV, 0);
			mipGenCB.DestMipIndex = pGfx->MipMapHeap->GetIndexFromOffset(pTexture->UAV, srcMip);

			pGfx->GraphicsCommandList->Get()->SetComputeRoot32BitConstants(0, 8, &mipGenCB, 0);

			pGfx->GraphicsCommandList->Get()->Dispatch(((destWidth + 8u - 1) / 8u), (destHeight + 8u - 1) / 8u, 1);

			CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(uavResource.Get());
			pGfx->GraphicsCommandList->Get()->ResourceBarrier(1, &uavBarrier);
		}

		pGfx->ExecuteCommandList(pGfx->GraphicsCommandList, pGfx->Device->GetGfxQueue(), true);
		// Reset UAV; they are not necessary after mip creation for now
		pTexture->UAV = {};
		pGfx->MipMapHeap->Reset();
		
	}

} // namespace mf

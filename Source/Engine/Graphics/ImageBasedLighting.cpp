#include "Skybox.hpp"
#include "ImageBasedLighting.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"
#include <AgilitySDK/d3dx12/d3dx12.h>
#include "TextureManager.hpp"
#include "ShaderCompiler.hpp"
#include <DirectXTex.h>
#include <directxtk12/ResourceUploadBatch.h>

namespace lde
{
	ImageBasedLighting::ImageBasedLighting(RHI::D3D12RHI* pRHI, Skybox* pSkybox, std::string_view Filepath)
		: m_Gfx(pRHI)
	{
		CreateComputeState();
		
		CreateHDRTexture(Filepath, pSkybox);

		CreateTextureCube(Filepath, pSkybox);
		
	}

	ImageBasedLighting::~ImageBasedLighting()
	{
		delete m_Equirect2CubeCS;

		SAFE_RELEASE(m_ComputeRS);
		SAFE_RELEASE(m_ComputePSO);
	}

	void ImageBasedLighting::CreateComputeState()
    {
		// Indices to shader views
		std::array<CD3DX12_ROOT_PARAMETER1, 3> parameters{};
		// SRV
		parameters.at(0).InitAsConstants(1, 0, 0);
		// UAV
		parameters.at(1).InitAsConstants(1, 1, 0);
		// sampling
		parameters.at(2).InitAsConstants(1, 2, 0);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc{};
		const CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
		signatureDesc.Init_1_1(static_cast<uint32_t>(parameters.size()), parameters.data(), 1, &computeSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);

		ID3DBlob* signature;
		ID3DBlob* error;

		RHI::DX_CALL(D3DX12SerializeVersionedRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_ComputeRS)));
		m_ComputeRS->SetName(L"[Image Based Lighting] Compute Root Signature");

		SAFE_DELETE(signature);
		SAFE_DELETE(error);

		auto& shaderManager = ShaderCompiler::GetInstance();
		m_Equirect2CubeCS = new Shader(shaderManager.Compile("Shaders/Sky/EquirectangularToCube.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
		// other shaders here

		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = m_ComputeRS.Get();
		psoDesc.CS = m_Equirect2CubeCS->Bytecode();
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_ComputePSO)));

    }

	void ImageBasedLighting::CreateHDRTexture(std::string_view Filepath, Skybox* pSkybox)
	{
		// https://github.com/michal-z/ImageBasedPBR/blob/master/Source/ImageBasedPBR.cpp#L389
		auto& textureManager = TextureManager::GetInstance();

		// Create texture from HDR image
		DirectX::ScratchImage scratchImage{};
		auto path = String::ToWide(Filepath);
		DirectX::LoadFromHDRFile(path.c_str(), nullptr, scratchImage);
		DirectX::TexMetadata metadata = scratchImage.GetMetadata();

		D3D12_RESOURCE_DESC desc{};
		desc.Format = metadata.format;
		desc.Width = static_cast<uint64>(metadata.width);
		desc.Height = static_cast<uint32>(metadata.height);
		desc.MipLevels = 6;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc = { 1, 0 };
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D)
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D)
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		pSkybox->Texture = new RHI::D3D12Texture();
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pSkybox->Texture->Texture)));
		pSkybox->Texture->Texture->SetName(L"Skybox Equirectangular Texture");

		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData = scratchImage.GetImages()->pixels;
		subresource.RowPitch = static_cast<int64>(scratchImage.GetImages()->rowPitch);
		subresource.SlicePitch = static_cast<int64>(scratchImage.GetImages()->slicePitch);

		const auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(scratchImage.GetImages()->slicePitch);

		Ref<ID3D12Resource> uploadResource;
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapUpload,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadResource)));

		DirectX::ScratchImage outMipChain{};
		DirectX::ResourceUploadBatch upload(m_Gfx->Device->GetDevice());
		upload.Begin();
		upload.Upload(pSkybox->Texture->Texture.Get(), 0, &subresource, 1);
		upload.Transition(pSkybox->Texture->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		upload.GenerateMips(pSkybox->Texture->Texture.Get());

		//m_Gfx->UploadResource(pSkybox->Texture->Texture, uploadResource, subresource);
		//m_Gfx->TransitResource(pSkybox->Texture->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		//textureManager.Generate2D(pSkybox->Texture);

		pSkybox->Texture->Width = desc.Width;
		pSkybox->Texture->Height = desc.Height;
		pSkybox->Texture->Format = desc.Format;
		pSkybox->Texture->MipLevels = desc.MipLevels;
		m_Gfx->Device->CreateSRV(pSkybox->Texture->Texture.Get(), pSkybox->Texture->SRV);

		auto finish{ upload.End(m_Gfx->Device->GetGfxQueue()->Get()) };
		finish.wait();

		SAFE_RELEASE(uploadResource);
	}

	void ImageBasedLighting::CreateTextureCube(std::string_view Filepath, Skybox* pSkybox)
	{	
		
		// UAV to transform into TextureCube
		Ref<ID3D12Resource> tempCube;
		// UAV type
		RHI::D3D12Descriptor cubeDescriptor;

		uint32 cubeResolution = 1024;
		if (pSkybox->Texture->Width < 1024)
		{
			cubeResolution = 512;
		}
		else if (pSkybox->Texture->Width >= 2048 && pSkybox->Texture->Width < 4096)
		{
			cubeResolution = 2048;
		}
		else if (pSkybox->Texture->Width >= 4096)
		{
			cubeResolution = 4096;
		}

		// Used for transforming texture
		D3D12_RESOURCE_DESC uavDesc{};
		uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		uavDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		uavDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		uavDesc.Width = cubeResolution;
		uavDesc.Height = cubeResolution;
		uavDesc.MipLevels = 6;
		uavDesc.DepthOrArraySize = 6;
		uavDesc.SampleDesc = { 1, 0 };
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&uavDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&tempCube)
		));
		tempCube->SetName(L"[Image Based Lighting] Environment Texture - pretransformed");
		m_Gfx->Device->CreateUAV(tempCube.Get(), cubeDescriptor, 6 * 6); // * 6
		m_Gfx->TransitResource(tempCube, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		
		auto dispatch([&](RHI::D3D12CommandList* pCmdList) {
			pCmdList->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
			pCmdList->Get()->SetComputeRootSignature(m_ComputeRS.Get());
			pCmdList->Get()->SetPipelineState(m_ComputePSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(0, pSkybox->Texture->SRV.Index(), 0);
			pCmdList->Get()->SetComputeRoot32BitConstant(1, cubeDescriptor.Index(), 0);
			pCmdList->Get()->Dispatch(cubeResolution / 32, cubeResolution / 32, 6);
			});
		dispatch(m_Gfx->Device->GetGfxCommandList());

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);

		pSkybox->TextureCube = new RHI::D3D12Texture();

		D3D12_RESOURCE_DESC textureCubeDesc{};
		textureCubeDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureCubeDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureCubeDesc.Width = cubeResolution;
		textureCubeDesc.Height = cubeResolution;
		textureCubeDesc.DepthOrArraySize = 6;
		textureCubeDesc.MipLevels = 6;
		textureCubeDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureCubeDesc.SampleDesc = { 1, 0 };
		textureCubeDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE,
			&textureCubeDesc, D3D12_RESOURCE_STATE_COMMON,
			nullptr, IID_PPV_ARGS(&pSkybox->TextureCube->Texture)));
		pSkybox->TextureCube->Texture->SetName(L"Skybox TextureCube");
		
		m_Gfx->TransitResource(tempCube, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_Gfx->TransitResource(pSkybox->TextureCube->Texture, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

		//m_Gfx->CopyResource(pSkybox->TextureCube->Texture, tempCube);

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
			
		for (uint32 arraySlice = 0; arraySlice < 6; ++arraySlice)
		{
			const UINT subresourceIndex = D3D12CalcSubresource(0, arraySlice, 0, 6, 6);
			auto dst = CD3DX12_TEXTURE_COPY_LOCATION{ pSkybox->TextureCube->Texture.Get(), subresourceIndex };
			auto src = CD3DX12_TEXTURE_COPY_LOCATION{ tempCube.Get(), subresourceIndex };
			m_Gfx->Device->GetGfxCommandList()->Get()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
		

		pSkybox->TextureCube->Width = textureCubeDesc.Width;
		pSkybox->TextureCube->Height = textureCubeDesc.Height;
		pSkybox->TextureCube->MipLevels = textureCubeDesc.MipLevels;
		
		m_Gfx->TransitResource(pSkybox->TextureCube->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		//m_Gfx->TransitResource(pSkybox->TextureCube->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		m_Gfx->Device->CreateSRV(pSkybox->TextureCube->Texture.Get(), pSkybox->TextureCube->SRV, 1, 1);
		TextureManager::GetInstance().Generate3D(pSkybox->TextureCube);
		
		//m_Gfx->TransitResource(pSkybox->TextureCube->Texture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		SAFE_RELEASE(tempCube);

	}

	void ImageBasedLighting::CreateDiffuseTexture()
	{
		const uint32 cubeResolution = 256;
		const uint32 mipLevels = 6;

		RHI::D3D12Descriptor uavDescriptor;
		m_Gfx->Device->Allocate(RHI::HeapType::eSRV, uavDescriptor, 6 * mipLevels);

		{
			auto desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, cubeResolution, cubeResolution, 6, mipLevels);
			m_Gfx->Device->GetDevice()->CreateCommittedResource(&RHI::D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&SpecularTexture->Texture));
			m_Gfx->Device->Allocate(RHI::HeapType::eSRV, SpecularTexture->SRV, 1);




		}


	}
} // namespace lde

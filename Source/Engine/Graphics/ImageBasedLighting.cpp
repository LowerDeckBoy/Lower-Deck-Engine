#include "Skybox.hpp"
#include "ImageBasedLighting.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"
//#include <AgilitySDK/d3dx12/d3dx12.h>
#include "TextureManager.hpp"
#include "ShaderCompiler.hpp"
#include <DirectXTex.h>
#include <directxtk12/ResourceUploadBatch.h>
#include <Core/Math.hpp>
#include <Core/Logger.hpp>

namespace lde
{
	constexpr int32 DISPATCH_X = 32;
	constexpr int32 DISPATCH_Y = 32;
	constexpr int32 DISPATCH_Z = 6;

	// Helper enum for Root Signature slots
	enum BindingSlot : uint32
	{
		eSRV = 0,
		eUAV = 1,
		eSampling = 2
	};

	ImageBasedLighting::ImageBasedLighting(RHI::D3D12RHI* pRHI, Skybox* pSkybox, std::string_view Filepath)
		: m_Gfx(pRHI)
	{
		CreateComputeStates();
		
		m_Gfx->Device->GetGfxCommandList()->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());

		CreateHDRTexture(Filepath, pSkybox);
		CreateTextureCube(pSkybox);
		CreateDiffuseTexture(pSkybox);
		CreateSpecularTexture(pSkybox);
		CreateBRDFTexture(pSkybox);

	}

	ImageBasedLighting::~ImageBasedLighting()
	{
		delete m_Shaders.Equirect2CubeCS;
		delete m_Shaders.DiffuseIrradianceCS;
		delete m_Shaders.SpecularCS;
		delete m_Shaders.BRDFLookUpCS;

		SAFE_RELEASE(m_Pipelines.BRDFLookUpPSO);
		SAFE_RELEASE(m_Pipelines.SpecularPSO);
		SAFE_RELEASE(m_Pipelines.DiffusePSO);
		SAFE_RELEASE(m_Pipelines.ComputePSO);
		SAFE_RELEASE(m_Pipelines.ComputeRS);
	}

	void ImageBasedLighting::CreateComputeStates()
	{
		// Common Root Signature
		{
			// Indices to shader views
			std::array<CD3DX12_ROOT_PARAMETER1, 3> parameters{};
			// SRV
			parameters.at(0).InitAsConstants(1, BindingSlot::eSRV, 0);
			// UAV
			parameters.at(1).InitAsConstants(1, BindingSlot::eUAV, 0);
			// Sampling; for Specular
			parameters.at(2).InitAsConstants(1, BindingSlot::eSampling, 0);

			const auto rootFlags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | 
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;
			
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc{};
			const CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
			signatureDesc.Init_1_1(static_cast<uint32_t>(parameters.size()), parameters.data(), 1, &computeSamplerDesc, rootFlags);

			ID3DBlob* signature;
			ID3DBlob* error;

			RHI::DX_CALL(D3DX12SerializeVersionedRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_Pipelines.ComputeRS)));
			m_Pipelines.ComputeRS->SetName(L"[Image Based Lighting] Compute Root Signature");

			SAFE_DELETE(signature);
			SAFE_DELETE(error);
		}
		
		// Shaders
		{
			auto& shaderManager = ShaderCompiler::GetInstance();
			m_Shaders.Equirect2CubeCS		= new Shader(shaderManager.Compile("Shaders/Sky/EquirectangularToCube.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
			m_Shaders.DiffuseIrradianceCS	= new Shader(shaderManager.Compile("Shaders/Sky/IrradianceCS.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
			m_Shaders.SpecularCS			= new Shader(shaderManager.Compile("Shaders/Sky/SpecularCS.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
			m_Shaders.BRDFLookUpCS			= new Shader(shaderManager.Compile("Shaders/Sky/SpecularBRDF.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
		}
		
		// Equirectangular to Cube
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = m_Pipelines.ComputeRS.Get();
			psoDesc.CS = m_Shaders.Equirect2CubeCS->Bytecode(); 
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.ComputePSO)));
		}
		
		// Diffuse irradiance
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = m_Pipelines.ComputeRS.Get();
			psoDesc.CS = m_Shaders.DiffuseIrradianceCS->Bytecode();
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.DiffusePSO)));
		}

		// Specular
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = m_Pipelines.ComputeRS.Get();
			psoDesc.CS = m_Shaders.SpecularCS->Bytecode();
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.SpecularPSO)));
		}

		// BRDF LUT
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = m_Pipelines.ComputeRS.Get();
			psoDesc.CS = m_Shaders.BRDFLookUpCS->Bytecode();
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.BRDFLookUpPSO)));
		}

	}

	void ImageBasedLighting::CreateHDRTexture(std::string_view Filepath, Skybox* pSkybox)
	{
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

		//DirectX::ScratchImage outMipChain{};
		//DirectX::ResourceUploadBatch upload(m_Gfx->Device->GetDevice());
		//upload.Begin();
		//upload.Upload(pSkybox->Texture->Texture.Get(), 0, &subresource, 1);
		//upload.Transition(pSkybox->Texture->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//upload.GenerateMips(pSkybox->Texture->Texture.Get());

		m_Gfx->UploadResource(pSkybox->Texture->Texture, uploadResource, subresource);
		m_Gfx->TransitResource(pSkybox->Texture->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		//textureManager.Generate2D(pSkybox->Texture);

		pSkybox->Texture->Width		= static_cast<uint32>(desc.Width);
		pSkybox->Texture->Height	= desc.Height;
		pSkybox->Texture->Format	= desc.Format;
		pSkybox->Texture->MipLevels = desc.MipLevels;
		m_Gfx->Device->CreateSRV(pSkybox->Texture->Texture.Get(), pSkybox->Texture->SRV);

		//auto finish{ upload.End(m_Gfx->Device->GetGfxQueue()->Get()) };
		//finish.wait();

		SAFE_RELEASE(uploadResource);
	}

	void ImageBasedLighting::CreateTextureCube(Skybox* pSkybox)
	{	
		// UAV to transform into TextureCube
		Ref<ID3D12Resource> tempCube;
		// UAV type
		RHI::D3D12Descriptor cubeDescriptor;

		// Determine resolution of output TextureCube based on input equirectangular map
		uint32 cubeResolution = 1024;
		if (pSkybox->Texture->Width <= 1024)
		{
			cubeResolution = 1024;
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
		m_Gfx->Device->CreateUAV(tempCube.Get(), cubeDescriptor, 0, 36);
		m_Gfx->TransitResource(tempCube, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		
		auto dispatch([&](RHI::D3D12CommandList* pCmdList) {
			pCmdList->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS.Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.ComputePSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eSRV, pSkybox->Texture->SRV.Index(), 0);
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eUAV, cubeDescriptor.Index(), 0);
			pCmdList->Get()->Dispatch(cubeResolution / DISPATCH_X, cubeResolution / DISPATCH_Y, DISPATCH_Z);
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

		m_Gfx->CopyResource(pSkybox->TextureCube->Texture, tempCube);
		
		//for (uint32 arraySlice = 0; arraySlice < 6; ++arraySlice)
		//{
		//	const UINT subresourceIndex = D3D12CalcSubresource(0, arraySlice, 0, 6, 6);
		//	auto dst = CD3DX12_TEXTURE_COPY_LOCATION{ pSkybox->TextureCube->Texture.Get(), subresourceIndex };
		//	auto src = CD3DX12_TEXTURE_COPY_LOCATION{ tempCube.Get(), subresourceIndex };
		//	m_Gfx->Device->GetGfxCommandList()->Get()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		//}
		
		pSkybox->TextureCube->Width		= static_cast<uint32>(textureCubeDesc.Width);
		pSkybox->TextureCube->Height	= textureCubeDesc.Height;
		pSkybox->TextureCube->MipLevels = textureCubeDesc.MipLevels;
		
		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		auto transit = CD3DX12_RESOURCE_BARRIER::Transition(pSkybox->TextureCube->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_Gfx->Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &transit);
		m_Gfx->Device->CreateSRV(pSkybox->TextureCube->Texture.Get(), pSkybox->TextureCube->SRV, 6, 1);
		TextureManager::GetInstance().Generate3D(pSkybox->TextureCube);

		SAFE_RELEASE(tempCube);

	}

	void ImageBasedLighting::CreateDiffuseTexture(Skybox* pSkybox)
	{
		const uint32 cubeResolution = 128; // 32 ?

		pSkybox->DiffuseTexture = new RHI::D3D12Texture();

		const auto format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		//const auto format = DXGI_FORMAT_R11G11B10_FLOAT;

		D3D12_RESOURCE_DESC desc{};
		desc.Format = format;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = cubeResolution;
		desc.Height = cubeResolution;
		desc.DepthOrArraySize = 6;
		desc.MipLevels = 1;
		desc.SampleDesc = { 1, 0 };
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault, 
			D3D12_HEAP_FLAG_NONE, 
			&desc, 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr, 
			IID_PPV_ARGS(&pSkybox->DiffuseTexture->Texture)));
		pSkybox->DiffuseTexture->Texture->SetName(L"[Image Based Lighting] Diffuse Irradiance Texture");
		m_Gfx->Device->Allocate(RHI::HeapType::eSRV, pSkybox->DiffuseTexture->SRV, 1);
		
		m_Gfx->Device->CreateUAV(pSkybox->DiffuseTexture->Texture.Get(), pSkybox->DiffuseTexture->UAV);
		
		const auto dispatch = [&](RHI::D3D12CommandList* pCmdList) {
			pCmdList->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS.Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.DiffusePSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eSRV, pSkybox->TextureCube->SRV.Index(), 0);
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eUAV, pSkybox->DiffuseTexture->UAV.Index(), 0);
			pCmdList->Get()->Dispatch(cubeResolution / DISPATCH_X, cubeResolution / DISPATCH_Y, DISPATCH_Z);
			};
		dispatch(m_Gfx->Device->GetGfxCommandList());

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		
		m_Gfx->Device->CreateSRV(pSkybox->DiffuseTexture->Texture.Get(), pSkybox->DiffuseTexture->SRV);

	}

	void ImageBasedLighting::CreateSpecularTexture(Skybox* pSkybox)
	{
		const uint32 cubeResolution = pSkybox->TextureCube->Width;
		const uint32 mipLevels = 6;

		pSkybox->SpecularTexture = new RHI::D3D12Texture();

		const auto format = DXGI_FORMAT_R16G16B16A16_FLOAT;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = format;
		desc.Width = cubeResolution;
		desc.Height = cubeResolution;
		desc.MipLevels = mipLevels;
		desc.DepthOrArraySize = 6;
		desc.SampleDesc = { 1, 0 };
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pSkybox->SpecularTexture->Texture)
		));
		pSkybox->SpecularTexture->Texture->SetName(L"[Image Based Lighting] Specular Texture");

		pSkybox->SpecularTexture->Width		= static_cast<uint32>(desc.Width);
		pSkybox->SpecularTexture->Height	= desc.Height;
		pSkybox->SpecularTexture->MipLevels = desc.MipLevels;
		pSkybox->SpecularTexture->Format	= format;

		//m_Gfx->TransitResource(pSkybox->TextureCube->Texture, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
		
		for (uint32 arraySlice = 0; arraySlice < 6; ++arraySlice)
		{
			const uint32 subresourceIndex = D3D12CalcSubresource(0, arraySlice, 0, mipLevels, 1);
			auto src = CD3DX12_TEXTURE_COPY_LOCATION{ pSkybox->TextureCube->Texture.Get(), subresourceIndex };
			auto dst = CD3DX12_TEXTURE_COPY_LOCATION{ pSkybox->SpecularTexture->Texture.Get(), subresourceIndex };
			m_Gfx->Device->GetGfxCommandList()->Get()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		
		m_Gfx->TransitResource(pSkybox->TextureCube->Texture, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_Gfx->TransitResource(pSkybox->SpecularTexture->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		const auto dispatch = [&](RHI::D3D12CommandList* pCmdList) {
			pCmdList->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS.Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.SpecularPSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eSRV, pSkybox->TextureCube->SRV.Index(), 0);
			
			const float deltaRoughness = 1.0f / std::max(static_cast<float>(mipLevels - 1), 1.0f);
			
			for (uint32 srcMip = 1; srcMip < 6; ++srcMip)
			{
				float sample = static_cast<float>(cubeResolution) / 2.0f;
				//float sample = 512.0f;
				const uint32 numGroups = std::max<uint32>(1, static_cast<uint32>(sample / 32.0f));

				for (uint32 arraySlice = 0; arraySlice < 6; ++arraySlice)
				{
					const float spmapRoughness = srcMip * deltaRoughness;
					pCmdList->Get()->SetComputeRoot32BitConstants(BindingSlot::eSampling, 1, &spmapRoughness, 0);
					
					m_Gfx->Device->CreateUAV(pSkybox->SpecularTexture->Texture.Get(), pSkybox->SpecularTexture->UAV, srcMip, 6);
					uint32 index = m_Gfx->Device->GetSRVHeap()->GetIndexFromOffset(pSkybox->SpecularTexture->UAV, arraySlice + 6);
					pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eUAV, index, 0);
					pCmdList->Get()->Dispatch(numGroups, numGroups, 6);

					sample /= 2.0f;
				}
			}
			};
		dispatch(m_Gfx->Device->GetGfxCommandList());

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		m_Gfx->Device->CreateSRV(pSkybox->SpecularTexture->Texture.Get(), pSkybox->SpecularTexture->SRV, 6, 1);

	}

	void ImageBasedLighting::CreateBRDFTexture(Skybox* pSkybox)
	{
		pSkybox->BRDFTexture = new RHI::D3D12Texture();

		const DXGI_FORMAT format = DXGI_FORMAT_R16G16_FLOAT;

		const uint32 resolution = 256;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = format;
		desc.Width = resolution;
		desc.Height = resolution;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc.SampleDesc = { 1, 0 };
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&pSkybox->BRDFTexture->Texture)
		));
		pSkybox->BRDFTexture->Texture->SetName(L"[Image Based Lighting] Specular BRDF LUT texture");

		m_Gfx->Device->CreateUAV(pSkybox->BRDFTexture->Texture.Get(), pSkybox->BRDFTexture->UAV);

		const auto dispatch = [&](RHI::D3D12CommandList* pCmdList){
			pCmdList->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS.Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.BRDFLookUpPSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eUAV, pSkybox->BRDFTexture->UAV.Index(), 0);
			pCmdList->Get()->Dispatch(resolution / DISPATCH_X, resolution / DISPATCH_Y, DISPATCH_Z);
		};
		dispatch(m_Gfx->Device->GetGfxCommandList());

		m_Gfx->TransitResource(pSkybox->BRDFTexture->Texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		m_Gfx->Device->CreateSRV(pSkybox->BRDFTexture->Texture.Get(), pSkybox->BRDFTexture->SRV);

	}

} // namespace lde

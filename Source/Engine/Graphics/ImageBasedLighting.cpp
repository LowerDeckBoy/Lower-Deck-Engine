#include "ImageBasedLighting.hpp"
#include "RHI/D3D12/D3D12RootSignature.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"
#include "ShaderCompiler.hpp"
#include "Skybox.hpp"
#include "TextureManager.hpp"
#include <AgilitySDK/d3dx12/d3dx12_resource_helpers.h>
#include <Core/Logger.hpp>
#include <Core/Math.hpp>
#include <stb/stb_image.h>

namespace lde
{
	constexpr int32 DISPATCH_X = 8;
	constexpr int32 DISPATCH_Y = 8;
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
		m_Pipelines.ComputeRS->Release();
	}

	void ImageBasedLighting::CreateComputeStates()
	{
		// Common Root Signature
		{
			m_Pipelines.ComputeRS = new RHI::D3D12RootSignature();
			m_Pipelines.ComputeRS->AddConstants(1, BindingSlot::eSRV, 0);
			m_Pipelines.ComputeRS->AddConstants(1, BindingSlot::eUAV, 0);
			m_Pipelines.ComputeRS->AddConstants(1, BindingSlot::eSampling, 0);
			m_Pipelines.ComputeRS->AddStaticSampler(0, 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
			RHI::DX_CALL(m_Pipelines.ComputeRS->Build(m_Gfx->Device.get(), RHI::PipelineType::eCompute));
		}
		
		// Shaders
		{
			auto& shaderManager = ShaderCompiler::GetInstance();
			m_Shaders.Equirect2CubeCS = new Shader(shaderManager.Compile("Shaders/Sky/EquirectangularToCube.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
			m_Shaders.DiffuseIrradianceCS = new Shader(shaderManager.Compile("Shaders/Sky/IrradianceCS.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
			m_Shaders.SpecularCS = new Shader(shaderManager.Compile("Shaders/Sky/SpecularCS.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
			m_Shaders.BRDFLookUpCS	= new Shader(shaderManager.Compile("Shaders/Sky/SpecularBRDF.hlsl", RHI::ShaderStage::eCompute, L"CSmain"));
		}
		
		// Equirectangular to Cube
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = m_Pipelines.ComputeRS->Get();
			psoDesc.CS = m_Shaders.Equirect2CubeCS->Bytecode(); 
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.ComputePSO)));
		}
		
		// Diffuse irradiance
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = m_Pipelines.ComputeRS->Get();
			psoDesc.CS = m_Shaders.DiffuseIrradianceCS->Bytecode();
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.DiffusePSO)));
		}

		// Specular
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
			psoDesc.pRootSignature = m_Pipelines.ComputeRS->Get();
			psoDesc.CS = m_Shaders.SpecularCS->Bytecode();
			RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.SpecularPSO)));
		}

		// BRDF LUT
		//{
		//	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
		//	psoDesc.pRootSignature = m_Pipelines.ComputeRS->Get();
		//	psoDesc.CS = m_Shaders.BRDFLookUpCS->Bytecode();
		//	RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_Pipelines.BRDFLookUpPSO)));
		//}

	}

	void ImageBasedLighting::CreateHDRTexture(std::string_view Filepath, Skybox* pSkybox)
	{
		int32 width,height = 0;
		stbi_ldr_to_hdr_scale(1.0f);
		stbi_ldr_to_hdr_gamma(2.2f);
		float* pixels = stbi_loadf(Filepath.data(), &width, &height, 0, STBI_rgb_alpha);

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = static_cast<uint64_t>(width);
		desc.Height = static_cast<uint32_t>(height);
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.MipLevels = 1;
		desc.DepthOrArraySize = 1;
		desc.SampleDesc = { 1, 0 };
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		pSkybox->Texture = new RHI::D3D12Texture();
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapDefault,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pSkybox->Texture->Texture)
		));

		D3D12_SUBRESOURCE_DATA subresource{};
		subresource.pData = pixels;
		subresource.RowPitch = static_cast<LONG_PTR>(desc.Width * 16u);
		subresource.SlicePitch = static_cast<LONG_PTR>(subresource.RowPitch * desc.Height);

		const auto uploadBuffer = CD3DX12_RESOURCE_DESC::Buffer(subresource.SlicePitch);

		Ref<ID3D12Resource> uploadResource;
		RHI::DX_CALL(m_Gfx->Device->GetDevice()->CreateCommittedResource(
			&RHI::D3D12Utility::HeapUpload,
			D3D12_HEAP_FLAG_NONE,
			&uploadBuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadResource.ReleaseAndGetAddressOf())
		));
		uploadResource->SetName(L"Texture Upload Resource");

		m_Gfx->UploadResource(pSkybox->Texture->Texture, uploadResource, subresource);
		m_Gfx->TransitResource(pSkybox->Texture->Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);

		m_Gfx->Device->CreateSRV(pSkybox->Texture->Texture.Get(), pSkybox->Texture->SRV, 1, 1);

		stbi_image_free(pixels);
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
		uavDesc.Format		= DXGI_FORMAT_R16G16B16A16_FLOAT;
		uavDesc.Dimension	= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		uavDesc.Flags		= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		uavDesc.Width		= cubeResolution;
		uavDesc.Height		= cubeResolution;
		uavDesc.MipLevels	= 6;
		uavDesc.DepthOrArraySize = 6;
		uavDesc.SampleDesc	= { 1, 0 };
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
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS->Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.ComputePSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eSRV, pSkybox->Texture->SRV.Index(), 0);
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eUAV, cubeDescriptor.Index(), 0);
			pCmdList->Get()->Dispatch(cubeResolution / DISPATCH_X, cubeResolution / DISPATCH_Y, DISPATCH_Z);
			});
		dispatch(m_Gfx->Device->GetGfxCommandList());
		//dispatch(m_Gfx->Device->GetComputeCommandList());

		//m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eCompute, true);
		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);

		pSkybox->TextureCube = new RHI::D3D12Texture();

		D3D12_RESOURCE_DESC textureCubeDesc{};
		textureCubeDesc.Format		= DXGI_FORMAT_R16G16B16A16_FLOAT;
		textureCubeDesc.Dimension	= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureCubeDesc.Width		= cubeResolution;
		textureCubeDesc.Height		= cubeResolution;
		textureCubeDesc.MipLevels	= 6;
		textureCubeDesc.DepthOrArraySize = 6;
		textureCubeDesc.Layout		= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureCubeDesc.SampleDesc	= { 1, 0 };
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
		//m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eCompute, true);
		
		m_Gfx->TransitResource(pSkybox->TextureCube->Texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE);

		m_Gfx->Device->CreateSRV(pSkybox->TextureCube->Texture.Get(), pSkybox->TextureCube->SRV, 6, 1);
		TextureManager::GetInstance().Generate3D(pSkybox->TextureCube);

		SAFE_RELEASE(tempCube);

	}

	void ImageBasedLighting::CreateDiffuseTexture(Skybox* pSkybox)
	{
		const uint32 cubeResolution = 64; // 256

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
		
		m_Gfx->Device->CreateUAV(pSkybox->DiffuseTexture->Texture.Get(), pSkybox->DiffuseTexture->UAV, 0, 1);
		
		const auto dispatch = [&](RHI::D3D12CommandList* pCmdList) {
			pCmdList->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS->Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.DiffusePSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eSRV, pSkybox->TextureCube->SRV.Index(), 0);
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eUAV, pSkybox->DiffuseTexture->UAV.Index(), 0);
			pCmdList->Get()->Dispatch(cubeResolution / DISPATCH_X, cubeResolution / DISPATCH_Y, DISPATCH_Z);
			};
		dispatch(m_Gfx->Device->GetGfxCommandList());

		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		
		m_Gfx->Device->CreateSRV(pSkybox->DiffuseTexture->Texture.Get(), pSkybox->DiffuseTexture->SRV, 1, 1);

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
		pSkybox->SpecularTexture->m_Format	= format;
		
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
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS->Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.SpecularPSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eSRV, pSkybox->TextureCube->SRV.Index(), 0);
			
			const float deltaRoughness = 1.0f / std::max(static_cast<float>(mipLevels - 1), 1.0f);
			
			for (uint32 srcMip = 1; srcMip < 6; ++srcMip)
			{
				float sample = static_cast<float>(cubeResolution) / 2.0f;
				//float sample = 512.0f;
				const uint32 numGroups = std::max<uint32>(1, static_cast<uint32>(sample / 8.0f));

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

		pSkybox->BRDF_LUT = TextureManager::GetInstance().Create(m_Gfx, "Assets/Textures/brdf_lut.png", false);
		//pSkybox->BRDF_LUT = TextureManager::GetInstance().Create(m_Gfx, "Assets/Textures/dfg.png", false);


		/*
		const DXGI_FORMAT format = DXGI_FORMAT_R16G16_FLOAT;
		const uint32 resolution = 512;

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

		m_Gfx->Device->CreateUAV(pSkybox->BRDFTexture->Texture.Get(), pSkybox->BRDFTexture->UAV, 0, 1);

		const auto dispatch = [&](RHI::D3D12CommandList* pCmdList){
			pCmdList->Get()->SetDescriptorHeaps(1, m_Gfx->Device->GetSRVHeap()->GetAddressOf());
			pCmdList->Get()->SetComputeRootSignature(m_Pipelines.ComputeRS->Get());
			pCmdList->Get()->SetPipelineState(m_Pipelines.BRDFLookUpPSO.Get());
			pCmdList->Get()->SetComputeRoot32BitConstant(BindingSlot::eUAV, pSkybox->BRDFTexture->UAV.Index(), 0);
			pCmdList->Get()->Dispatch(resolution / DISPATCH_X, resolution / DISPATCH_Y, 1);
		};
		dispatch(m_Gfx->Device->GetGfxCommandList());

		m_Gfx->TransitResource(pSkybox->BRDFTexture->Texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_Gfx->Device->ExecuteCommandList(RHI::CommandType::eGraphics, true);
		m_Gfx->Device->CreateSRV(pSkybox->BRDFTexture->Texture.Get(), pSkybox->BRDFTexture->SRV, 1, 1);
		*/
	}

} // namespace lde

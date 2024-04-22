#include "Scene/Model/Model.hpp"
#include "D3D12Device.hpp"
#include "D3D12Raytracing.hpp"
#include "D3D12Utility.hpp"
#include "D3D12RootSignature.hpp"
#include "D3D12Texture.hpp"
#include "Core/Math.hpp"
#include "Core/Logger.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include <AgilitySDK/d3dx12/d3dx12.h>
#include "Platform/Window.hpp"

namespace lde::RHI
{
	enum GlobalRootSignature
	{
		SceneBVH,
		TopLevel,
		Camera
	};

	D3D12Raytracing::D3D12Raytracing(D3D12Device* pDevice)
		: m_Device(pDevice)
	{
		BuildShaders();
		CreateRootSignature();
		
		
	}

	D3D12Raytracing::~D3D12Raytracing()
	{
		for (auto& blas : m_BLASes)
		{
			SAFE_RELEASE(blas.ScratchBuffer);
			SAFE_RELEASE(blas.ResultBuffer);
		}

		SAFE_RELEASE(TLAS.ScratchBuffer);
		SAFE_RELEASE(TLAS.ResultBuffer);
		SAFE_RELEASE(TLAS.InstanceDescsBuffer);
		
		RootSignatures.Miss->Release();
		RootSignatures.ClosestHit->Release();

		m_RootSignature->Release();
		m_SceneBVH->Release();

		LOG_INFO("DXR context released.");
	}

	void D3D12Raytracing::BuildShaders()
	{
		auto& shaderCompiler = ShaderCompiler::GetInstance();

		Shaders.RayGen = new Shader(shaderCompiler.Compile("Shaders/Raytracing/RayGen.hlsl", ShaderStage::eRaytracing, L"RayGen"));
		Shaders.ClosestHit = new Shader(shaderCompiler.Compile("Shaders/Raytracing/ClosestHit.hlsl", ShaderStage::eClosestHit, L"ClosestHit"));
		Shaders.Miss = new Shader(shaderCompiler.Compile("Shaders/Raytracing/Miss.hlsl", ShaderStage::eMiss, L"Miss"));

	}

	void D3D12Raytracing::CreateRootSignature()
	{
		m_RootSignature = new D3D12RootSignature();

		// 0. SceneBVH
		// 1. Top Level
		m_RootSignature->AddConstants(2, 0, 0);
		//m_RootSignature->AddUAV(0);
		//m_RootSignature->AddSRV(0);
		// Camera
		//m_RootSignature->AddCBV(0);
		DX_CALL(m_RootSignature->Build(m_Device, PipelineType::eGraphics, "Raytracing Root Signature"));

		// Miss RS
		{
			RootSignatures.Miss = new D3D12RootSignature();
			RootSignatures.Miss->SetLocal();
			DX_CALL(RootSignatures.Miss->Build(m_Device, PipelineType::eGraphics, "Raytracing Miss Root Signature"));
		}

		// ClosestHit RS
		{
			RootSignatures.ClosestHit = new D3D12RootSignature();
			RootSignatures.ClosestHit->AddConstants(sizeof(XMFLOAT4), 0, 1);
			RootSignatures.ClosestHit->SetLocal();
			DX_CALL(RootSignatures.ClosestHit->Build(m_Device, PipelineType::eGraphics, "Raytracing ClosestHit Root Signature"));
		}

	}

	void D3D12Raytracing::CreateSceneUAV()
	{
		m_SceneBVH = new D3D12Texture();

		D3D12_RESOURCE_DESC desc{};
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = static_cast<uint64>(Window::Width);
		desc.Height = Window::Height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc.SampleDesc = { 1, 0 };
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_SceneBVH->Texture)));
		SET_D3D12_NAME(m_SceneBVH->Texture, "Raytracing SceneBVH");
		
		m_Device->GetSRVHeap()->Allocate(m_SceneBVH->UAV);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		m_Device->GetDevice()->CreateUnorderedAccessView(m_SceneBVH->Texture.Get(), nullptr, &uavDesc, m_SceneBVH->UAV.GetCpuHandle());

		m_Device->GetSRVHeap()->Allocate(m_SceneBVH->SRV);
		D3D12_SHADER_RESOURCE_VIEW_DESC sceneSrvDesc{};
		sceneSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sceneSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		sceneSrvDesc.Texture2D.MipLevels = 1;
		sceneSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		m_Device->GetDevice()->CreateShaderResourceView(m_SceneBVH->Texture.Get(), &sceneSrvDesc, m_SceneBVH->SRV.GetCpuHandle());

		m_Device->GetSRVHeap()->Allocate(TLAS.SRV);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = TLAS.ResultBuffer->GetGPUVirtualAddress();
		m_Device->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, TLAS.SRV.GetCpuHandle());

	}

	void D3D12Raytracing::CreateStateObject()
	{
		//std::vector<D3D12_STATE_SUBOBJECT> subobjects{  };

		CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		// RayGen Shader
		{
			auto raygenLib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			const auto raygenBytecode = Shaders.RayGen->Bytecode();
			raygenLib->SetDXILLibrary(&raygenBytecode);
			raygenLib->DefineExport(L"RayGen");
		}

		// Miss Shader
		{
			auto missLib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			const auto missBytecode = Shaders.Miss->Bytecode();
			missLib->SetDXILLibrary(&missBytecode);
			missLib->DefineExport(L"Miss");
			
		}

		// Closest Hit Shader
		{
			auto hitLib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			const auto hitBytecode = Shaders.ClosestHit->Bytecode();
			hitLib->SetDXILLibrary(&hitBytecode);
			hitLib->DefineExport(L"ClosestHit");
		}

		// HitGroup
		{
			auto hitGroup{ raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>() };
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
			hitGroup->SetHitGroupExport(L"HitGroup");
			hitGroup->SetClosestHitShaderImport(L"ClosestHit");
		}

		// Shader Config
		{
			auto shaderConfig{ raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>() };
			m_PayloadSize = 16;
			m_AttributeSize = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;
			shaderConfig->Config(m_PayloadSize, m_AttributeSize);
			
		}

		auto pipelineConfig{ raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>() };
		pipelineConfig->Config(m_MaxRecursiveDepth);

		auto missSignature{ raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>() };
		missSignature->SetRootSignature(RootSignatures.Miss->Get());

		auto missAssociation{ raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>() };
		{
			missAssociation->SetSubobjectToAssociate(*missSignature);
			missAssociation->AddExport(L"Miss");
		}

		// aka ClosestHit Local Root Signature
		auto hitSignature{ raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>() };
		hitSignature->SetRootSignature(RootSignatures.ClosestHit->Get());

		auto hitAssociation{ raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>() };
		{
			hitAssociation->SetSubobjectToAssociate(*hitSignature);
			std::vector<LPCWSTR> exports{ L"HitGroup" };
			hitAssociation->AddExports(exports.data(), static_cast<uint32_t>(exports.size()));
		}

		auto globalRootSignature{ raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>() };
		globalRootSignature->SetRootSignature(m_RootSignature->Get());

		DX_CALL(m_Device->GetDevice()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(m_StateObject.ReleaseAndGetAddressOf())));
		DX_CALL(m_StateObject->QueryInterface(&m_StateObjectProperties));
		SET_D3D12_NAME(m_StateObject, "Raytracing State Object");
		
	}

	void D3D12Raytracing::BuildShaderTable()
	{
		constexpr uint32_t shaderIdentifierSize{ D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES };

		void* rayGenIdentifier	= m_StateObjectProperties.Get()->GetShaderIdentifier(L"RayGen");
		void* missIdentifier	= m_StateObjectProperties.Get()->GetShaderIdentifier(L"Miss");
		void* hitIdentifier		= m_StateObjectProperties.Get()->GetShaderIdentifier(L"HitGroup");

		// Raygen
		{
			ShaderTables.RayGen.Create(m_Device->GetDevice(), 1, shaderIdentifierSize, L"RayGen Shader Table");
			TableRecord record(rayGenIdentifier, shaderIdentifierSize, nullptr, shaderIdentifierSize);

			ShaderTables.RayGen.AddRecord(record);
		}
		// Miss
		{
			ShaderTables.Miss.Create(m_Device->GetDevice(), 1, shaderIdentifierSize, L"Miss Shader Table");
			TableRecord record(missIdentifier, shaderIdentifierSize, nullptr, shaderIdentifierSize);

			ShaderTables.Miss.AddRecord(record);
		}
		// ClosestHit
		{
			ShaderTables.ClosestHit.Create(m_Device->GetDevice(), 1, shaderIdentifierSize, L"Closest Shader Table");
			TableRecord record(hitIdentifier, shaderIdentifierSize, nullptr, shaderIdentifierSize);

			ShaderTables.ClosestHit.AddRecord(record);
		}

	}

	void D3D12Raytracing::DispatchRaytrace()
	{
		const auto dispatch = [&](ID3D12GraphicsCommandList8* pCommandList, const D3D12_DISPATCH_RAYS_DESC& Desc) {

			pCommandList->SetComputeRootSignature(m_RootSignature->Get());
			struct indices{ uint32 Out; uint32 Top; } data{ .Out = m_SceneBVH->UAV.Index(), .Top = TLAS.SRV.Index() };
			pCommandList->SetComputeRoot32BitConstants(0, 2,  &data, 0);
			
			pCommandList->SetPipelineState1(m_StateObject.Get());
			pCommandList->DispatchRays(&Desc);
			};

		D3D12_DISPATCH_RAYS_DESC dispatchDesc{};
		{
			// RayGen
			dispatchDesc.RayGenerationShaderRecord.StartAddress = ShaderTables.RayGen.GetAddressOf();
			dispatchDesc.RayGenerationShaderRecord.SizeInBytes  = ShaderTables.RayGen.GetShaderRecordSize();
			// Miss
			dispatchDesc.MissShaderTable.StartAddress = ShaderTables.Miss.GetAddressOf();
			dispatchDesc.MissShaderTable.SizeInBytes = static_cast<uint64_t>(ShaderTables.Miss.GetShaderRecordSize());
			dispatchDesc.MissShaderTable.StrideInBytes = static_cast<uint64_t>(ShaderTables.Miss.Stride());
			//// ClosestHit
			dispatchDesc.HitGroupTable.StartAddress = ShaderTables.ClosestHit.GetAddressOf();
			dispatchDesc.HitGroupTable.SizeInBytes = static_cast<uint64_t>(ShaderTables.ClosestHit.GetShaderRecordSize() * ShaderTables.ClosestHit.GetRecordsCount());
			dispatchDesc.HitGroupTable.StrideInBytes = static_cast<uint64_t>(ShaderTables.ClosestHit.Stride());
			//// Output dimensions
			dispatchDesc.Width  = static_cast<uint32_t>(Window::Width);
			dispatchDesc.Height = static_cast<uint32_t>(Window::Height);
			// Primary Rays
			dispatchDesc.Depth = m_MaxRecursiveDepth;
		}

		// Execute
		dispatch(m_Device->GetGfxCommandList()->Get(), dispatchDesc);
	}

	void D3D12Raytracing::CreateTLAS()
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc{};
		prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		prebuildDesc.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		prebuildDesc.NumDescs = static_cast<uint32>(TLAS.Instances.size());
		
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		m_Device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &prebuildInfo);

		for (auto& blas : m_BLASes)
		{
			TLAS.AddInstance(blas.ResultBuffer, XMMatrixIdentity(), 0, 0);
		}

		TLAS.ScratchSize = ALIGN(prebuildInfo.ScratchDataSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		TLAS.ResultSize  = ALIGN(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		TLAS.InstanceDescsSize = ALIGN(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * static_cast<uint64>(TLAS.Instances.size()), D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

		const auto scratchDesc = CreateBufferDesc(TLAS.ScratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &scratchDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&TLAS.ScratchBuffer)));
		SET_D3D12_NAME(TLAS.ScratchBuffer, "DXR TLAS Scratch Buffer");

		const auto resultDesc  = CreateBufferDesc(TLAS.ResultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &resultDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&TLAS.ResultBuffer)));
		SET_D3D12_NAME(TLAS.ResultBuffer, "DXR TLAS Result Buffer");

		const auto instancesDesc = CreateBufferDesc(TLAS.InstanceDescsSize, D3D12_RESOURCE_FLAG_NONE);
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapUpload, D3D12_HEAP_FLAG_NONE, &instancesDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&TLAS.InstanceDescsBuffer)));
		SET_D3D12_NAME(TLAS.InstanceDescsBuffer, "DXR TLAS Instance Descs Buffer");

		m_Device->ExecuteCommandList(CommandType::eGraphics, true);

		D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs{};
		DX_CALL(TLAS.InstanceDescsBuffer.Get()->Map(0, nullptr, reinterpret_cast<void**>(&instanceDescs)));

		for (uint32 instance = 0; instance < TLAS.Instances.size(); instance++)
		{
			instanceDescs[instance].AccelerationStructure = TLAS.Instances.at(instance).BottomLevel->GetGPUVirtualAddress();
			instanceDescs[instance].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			instanceDescs[instance].InstanceMask = 0xFF;
			instanceDescs[instance].InstanceID = TLAS.Instances.at(instance).InstanceGroup;
			instanceDescs[instance].InstanceContributionToHitGroupIndex = TLAS.Instances.at(instance).HitGroupID;

			DirectX::XMMATRIX transform = DirectX::XMMatrixTranspose(TLAS.Instances.at(instance).Matrix);
			std::memcpy(instanceDescs[instance].Transform, &transform, sizeof(instanceDescs[instance].Transform));

		}
		TLAS.InstanceDescsBuffer.Get()->Unmap(0, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
		buildDesc.Inputs = prebuildDesc;
		buildDesc.ScratchAccelerationStructureData = TLAS.ScratchBuffer->GetGPUVirtualAddress();
		buildDesc.DestAccelerationStructureData = TLAS.ResultBuffer->GetGPUVirtualAddress();
		buildDesc.SourceAccelerationStructureData = 0;

		m_Device->GetGfxCommandList()->Get()->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

		D3D12_RESOURCE_BARRIER uavBarrier{};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = TLAS.ResultBuffer.Get();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		m_Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &uavBarrier);

	}

	void D3D12Raytracing::AddBLAS(Model* pModel)
	{
		const auto* vertexBuffer = m_Device->Buffers.at(pModel->VertexBuffer);
		const auto* indexBuffer  = m_Device->Buffers.at(pModel->IndexBuffer);

		D3D12RaytracingBLAS blas{};
		
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGpuAddress();
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexBuffer->GetDesc().Stride;
		geometryDesc.Triangles.VertexCount = vertexBuffer->GetDesc().Count;

		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometryDesc.Triangles.IndexCount = indexBuffer->GetDesc().Count;
		geometryDesc.Triangles.IndexBuffer = indexBuffer->GetGpuAddress();

		geometryDesc.Triangles.Transform3x4 = 0;
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.NumDescs = 1;
		inputs.pGeometryDescs = &geometryDesc;
		
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
		m_Device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

		prebuildInfo.ScratchDataSizeInBytes   = ALIGN(prebuildInfo.ScratchDataSizeInBytes,   D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
		prebuildInfo.ResultDataMaxSizeInBytes = ALIGN(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

		blas.ScratchSize = prebuildInfo.ScratchDataSizeInBytes;
		blas.ResultSize  = prebuildInfo.ResultDataMaxSizeInBytes;

		const auto scratchBufferDesc = CreateBufferDesc(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&blas.ScratchBuffer)));
		
		const auto resultBufferDesc = CreateBufferDesc(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&blas.ResultBuffer)));

		//m_Device->ExecuteCommandList(CommandType::eGraphics, true);
		
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
		buildDesc.Inputs = inputs;
		buildDesc.ScratchAccelerationStructureData	= blas.ScratchBuffer->GetGPUVirtualAddress();
		buildDesc.DestAccelerationStructureData		= blas.ResultBuffer->GetGPUVirtualAddress();
		buildDesc.SourceAccelerationStructureData	= 0;
		
		m_Device->GetGfxCommandList()->Get()->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
		
		D3D12_RESOURCE_BARRIER uavBarrier{};
		uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		uavBarrier.UAV.pResource = blas.ResultBuffer.Get();
		uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		m_Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &uavBarrier);

		//m_Device->ExecuteCommandList(CommandType::eGraphics, true);
		
		m_BLASes.emplace_back(blas);

	}

	void D3D12RaytracingTLAS::AddInstance(Ref<ID3D12Resource> pBottomLevel, DirectX::XMMATRIX Matrix, uint32 InstanceGroup, uint32 HitGroupID)
	{
		Instances.emplace_back(Instance{ .BottomLevel = pBottomLevel, .Matrix = Matrix, .InstanceGroup = InstanceGroup, .HitGroupID = HitGroupID });
	}

	TableRecord::TableRecord(void* pIdentifier, uint32_t Size)
	{
		m_Identifier.pData = pIdentifier;
		m_Identifier.Size = Size;

		TotalSize += Size;
	}

	TableRecord::TableRecord(void* pIdentifier, uint32_t Size, void* pLocalRootArgs, uint32_t ArgsSize)
	{
		m_Identifier.pData = pIdentifier;
		m_Identifier.Size = Size;
		m_LocalRootArgs.pData = pLocalRootArgs;
		m_LocalRootArgs.Size = ArgsSize;

		TotalSize += Size + ArgsSize;
	}

	void TableRecord::CopyTo(void* pDestination)
	{
		uint8_t* pByteDestination{ static_cast<uint8_t*>(pDestination) };
		std::memcpy(pByteDestination, m_Identifier.pData, m_Identifier.Size);
		if (m_LocalRootArgs.pData != nullptr)
			std::memcpy(pByteDestination + m_Identifier.Size, m_LocalRootArgs.pData, m_LocalRootArgs.Size);
	}

	ShaderTable::~ShaderTable()
	{
		Release();
	}

	void ShaderTable::Create(ID3D12Device8* pDevice, uint32_t NumShaderRecord, uint32_t ShaderRecordSize, const std::wstring& DebugName)
	{
		m_ShaderRecordSize = ALIGN(ShaderRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		m_Records.reserve(NumShaderRecord);

		const uint32_t bufferSize{ NumShaderRecord * m_ShaderRecordSize };
		const auto uploadHeap{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };

		auto desc = CreateBufferDesc(bufferSize);
		DX_CALL(pDevice->CreateCommittedResource(&D3D12Utility::HeapUpload, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Storage)));
		
		const CD3DX12_RANGE range(0, 0);
		DX_CALL(m_Storage->Map(0, &range, reinterpret_cast<void**>(&m_MappedData)));

		if (!DebugName.empty())
			SetTableName(DebugName);
	}

	void ShaderTable::AddRecord(TableRecord& Record)
	{
		m_Records.push_back(Record);
		Record.CopyTo(m_MappedData);
		//m_MappedData += ALIGN(m_ShaderRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		m_MappedData += m_ShaderRecordSize;

	}

	void ShaderTable::SetStride(uint32_t Stride)
	{
		m_Stride = Stride;
	}

	void ShaderTable::CheckAlignment()
	{
		uint32_t max = std::max({ m_Records.data()->TotalSize });

		for (auto& record : m_Records)
			record.TotalSize = max;
		//record.TotalSize = ALIGN(max, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

		m_Stride = ALIGN(max, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

		m_Storage->Unmap(0, nullptr);
	}

	void ShaderTable::SetTableName(std::wstring Name)
	{
		m_Storage.Get()->SetName(Name.c_str());
	}

	void ShaderTable::Release()
	{
		SAFE_RELEASE(m_Storage);

		m_Records.clear();

		if (m_MappedData)
		{
			m_MappedData = nullptr;
			delete m_MappedData;
		}
	}

	void D3D12StateObjectBuilder::AddRayGen(Shader* pShader)
	{
		D3D12_DXIL_LIBRARY_DESC desc{};
		desc.DXILLibrary = pShader->Bytecode();
		desc.NumExports = 1;

		D3D12_EXPORT_DESC exportDesc{};
		exportDesc.Name = L"RayGen";
		exportDesc.ExportToRename = L"RayGen";
		exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

		desc.pExports = &exportDesc;

		D3D12_STATE_SUBOBJECT subobject{};
		subobject.pDesc = &desc;
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;

		Subobjects.push_back(subobject);
	}

	void D3D12StateObjectBuilder::AddClosestHit(Shader* pShader)
	{
		D3D12_DXIL_LIBRARY_DESC desc{};
		desc.DXILLibrary = pShader->Bytecode();
		desc.NumExports = 1;

		D3D12_EXPORT_DESC exportDesc{};
		exportDesc.Name = L"ClosestHit";
		exportDesc.ExportToRename = L"ClosestHit";
		exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

		desc.pExports = &exportDesc;

		D3D12_STATE_SUBOBJECT subobject{};
		subobject.pDesc = &desc;
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;

		Subobjects.push_back(subobject);
	}

} // namespace lde::RHI
#include "Scene/Model/Model.hpp"
#include "D3D12Device.hpp"
#include "D3D12Raytracing.hpp"
#include "D3D12Utility.hpp"
#include "Core/Math.hpp"

namespace lde::RHI
{
	D3D12Raytracing::D3D12Raytracing(D3D12Device* pDevice)
		: m_Device(pDevice)
	{
	}

	void D3D12Raytracing::AddBLAS(Model* pModel)
	{
		const auto* vertexBuffer = m_Device->Buffers.at(pModel->VertexBuffer);
		const auto* indexBuffer  = m_Device->Buffers.at(pModel->IndexBuffer);

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

		prebuildInfo.ScratchDataSizeInBytes   = ALIGN(prebuildInfo.ScratchDataSizeInBytes,   D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		prebuildInfo.ResultDataMaxSizeInBytes = ALIGN(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		const auto scratchBufferDesc = CreateBufferDesc(prebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&ScratchBuffer)));
		
		const auto resultBufferDesc = CreateBufferDesc(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		DX_CALL(m_Device->GetDevice()->CreateCommittedResource(&D3D12Utility::HeapDefault, D3D12_HEAP_FLAG_NONE, &scratchBufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&ResultBuffer)));

		m_Device->ExecuteCommandList(CommandType::eGraphics, true);
		
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
		buildDesc.Inputs = inputs;
		buildDesc.ScratchAccelerationStructureData	= ScratchBuffer->GetGPUVirtualAddress();
		buildDesc.DestAccelerationStructureData		= ResultBuffer->GetGPUVirtualAddress();
		buildDesc.SourceAccelerationStructureData	= 0;
		
		m_Device->GetGfxCommandList()->Get()->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
		//
		//D3D12_RESOURCE_BARRIER uavBarrier{};
		//uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		//uavBarrier.UAV.pResource = ResultBuffer.Get();
		//uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		//m_Device->GetGfxCommandList()->Get()->ResourceBarrier(1, &uavBarrier);

		//m_Device->ExecuteCommandList(CommandType::eGraphics, true);
		

	}
} // namespace lde::RHI

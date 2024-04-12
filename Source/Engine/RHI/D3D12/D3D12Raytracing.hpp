#pragma once

#include <AgilitySDK/d3d12.h>
#include <Graphics/ShaderCompiler.hpp>

namespace lde::RHI
{
	class D3D12Device;

	class D3D12RaytracingBLAS
	{
	public:
		D3D12RaytracingBLAS(D3D12Device* pDevice);
		~D3D12RaytracingBLAS();

		//void BuildGeometry(Model* pModel);

		ID3D12Resource* GetScratchBuffer() { return m_ScratchBuffer.Get(); }
		ID3D12Resource* GetResultBuffer()  { return m_ResultBuffer.Get();  }

	private:
		Ref<ID3D12Resource> m_ScratchBuffer;
		Ref<ID3D12Resource> m_ResultBuffer;

		uint64 m_ScratchSize = 0;
		uint64 m_ResultSize = 0;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE m_Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	};

	class D3D12RaytracingTLAS
	{
	public:

	};


	class D3D12ShaderTable
	{

	};

	class D3D12Raytracing
	{
	public:
		D3D12Raytracing(D3D12Device* pDevice);
		~D3D12Raytracing();

		void Release();

		void BuildShaders();


	private:
		D3D12Device* m_Device = nullptr; // Parent Device

		std::vector<D3D12RaytracingBLAS> m_BLASes;
		std::unique_ptr<D3D12RaytracingTLAS> m_TLAS;


		std::unique_ptr<Shader> m_RayGenShader;
		std::unique_ptr<Shader> m_ClosestHitShader;
		std::unique_ptr<Shader> m_MissShader;

		// Pipeline

	};
} // namespace lde::RHI

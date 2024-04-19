#pragma once

#include <AgilitySDK/d3d12.h>
#include <Graphics/ShaderCompiler.hpp>

namespace lde
{
	class Model;
}

namespace lde::RHI
{
	class D3D12Device;

	class D3D12Raytracing
	{
	public:
		D3D12Raytracing(D3D12Device* pDevice);
		~D3D12Raytracing();

		void Release();

		void AddTLAS();
		void AddBLAS(Model* pModel);

		void BuildShaders();

		Ref<ID3D12Resource> ScratchBuffer;
		Ref<ID3D12Resource> ResultBuffer;
	private:
		D3D12Device* m_Device = nullptr; // Parent Device

		struct 
		{
			Shader* RayGen;
			Shader* ClosestHit;
			Shader* Miss;
		} Shaders;

		// Pipeline

	};

	class D3D12ShaderTable
	{

	};

} // namespace lde::RHI

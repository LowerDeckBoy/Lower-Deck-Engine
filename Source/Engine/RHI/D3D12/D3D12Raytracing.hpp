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

	class D3D12RaytracingBLAS
	{
	public:
		D3D12RaytracingBLAS() = default;

		Ref<ID3D12Resource> ScratchBuffer;
		Ref<ID3D12Resource> ResultBuffer;
		uint64 ScratchSize = 0;
		uint64 ResultSize = 0;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	};

	class D3D12RaytracingTLAS
	{
	public:
		D3D12RaytracingTLAS() = default;

		void Create();

		struct Instance
		{
			Ref<ID3D12Resource> BottomLevel;
			DirectX::XMMATRIX Matrix;
			uint32 InstanceGroup;
			uint32 HitGroupID;
		};

		void AddInstance(Ref<ID3D12Resource> pBottomLevel, DirectX::XMMATRIX Matrix, uint32 InstanceGroup, uint32 HitGroupID);

		std::vector< Instance> Instances;

		Ref<ID3D12Resource> ScratchBuffer;
		Ref<ID3D12Resource> ResultBuffer;
		Ref<ID3D12Resource> InstanceDescsBuffer;

		uint64 ScratchSize = 0;
		uint64 ResultSize = 0;
		uint64 InstanceDescsSize = 0;

		D3D12Descriptor SRV;

	};


	class TableRecord
	{
	public:
		TableRecord(void* pIdentifier, uint32_t Size);
		TableRecord(void* pIdentifier, uint32_t Size, void* pLocalRootArgs, uint32_t ArgsSize);

		void CopyTo(void* pDestination);
		void SetAlignment(uint32_t Alignment);

		struct Identifier
		{
			void* pData = nullptr;
			uint32_t Size = 0;
		};

		Identifier m_Identifier;
		Identifier m_LocalRootArgs;

		// For using more then one record in a ShaderTable
		// requires aligment for largest record size
		uint32_t TotalSize = 0;

	};

	class ShaderTable
	{
	public:
		ShaderTable() {}
		~ShaderTable();

		void Create(ID3D12Device8* pDevice, uint32_t NumShaderRecord, uint32_t ShaderRecordSize, const std::wstring& DebugName = L"");

		void AddRecord(TableRecord& Record);

		void SetStride(uint32_t Stride);
		void CheckAlignment();

		void SetTableName(std::wstring Name);

		void Release();

		inline uint32_t GetRecordsCount() { return static_cast<uint32_t>(m_Records.size()); }
		inline uint32_t Stride() { return m_Stride; }

		inline uint32_t GetShaderRecordSize() { return m_ShaderRecordSize; }
		inline ID3D12Resource* GetStorage() { return m_Storage.Get(); }
		inline const D3D12_GPU_VIRTUAL_ADDRESS GetAddressOf() const { return m_Storage.Get()->GetGPUVirtualAddress(); }

	private:
		Ref<ID3D12Resource> m_Storage;
		uint8_t* m_MappedData = nullptr;
		uint32_t m_ShaderRecordSize = 0;
		uint32_t m_Stride = 0;

		std::vector<TableRecord> m_Records;

	};

	class D3D12Raytracing
	{
	public:
		D3D12Raytracing(D3D12Device* pDevice);
		~D3D12Raytracing();

		void Release();

		void DispatchRaytrace();

		void CreateTLAS();
		void AddBLAS(Model* pModel);

		void BuildShaders();
		void CreateRootSignature();
		void CreateSceneUAV();
		// https://github.com/acmarrs/IntroToDXR/blob/master/src/Graphics.cpp#L1163
		void CreateStateObject();
		void BuildShaderTable();

		D3D12RaytracingTLAS TLAS;
		D3D12Texture* m_SceneBVH;
	private:
		D3D12Device* m_Device = nullptr; // Parent Device

		// Output resource; UAV.

		D3D12RootSignature* m_RootSignature;

		Ref<ID3D12StateObject> m_StateObject;
		Ref<ID3D12StateObjectProperties> m_StateObjectProperties;

		struct 
		{
			Shader* RayGen;
			Shader* ClosestHit;
			Shader* Miss;
		} Shaders;
		
		struct 
		{
			D3D12RootSignature* RayGen;
			D3D12RootSignature* ClosestHit;
			D3D12RootSignature* Miss;
		} RootSignatures;

		struct
		{
			ShaderTable RayGen;
			ShaderTable Miss;
			ShaderTable ClosestHit;
		} ShaderTables;

		std::vector<D3D12RaytracingBLAS> m_BLASes;

		uint32 m_PayloadSize = 0;
		uint32 m_AttributeSize = 0;
		// 1 -> Primary rays
		// 2 -> Shadow rays
		uint32 m_MaxRecursiveDepth = 1;

	};


	class D3D12StateObjectBuilder
	{
	public:
		D3D12StateObjectBuilder();
		~D3D12StateObjectBuilder();

		void AddRayGen(Shader* pShader);
		void AddClosestHit(Shader* pShader);
		void AddMiss(Shader* pShader);

		void AddRecord();

		HRESULT Build(D3D12Device* pDevice, uint32& Output);

	private:
		Ref<ID3D12StateObject>				m_StateObject;
		Ref<ID3D12StateObjectProperties>	m_StateObjectProperties;

		std::vector<D3D12_STATE_SUBOBJECT> Subobjects;
		D3D12_STATE_OBJECT_DESC m_Desc{};

	};

} // namespace lde::RHI

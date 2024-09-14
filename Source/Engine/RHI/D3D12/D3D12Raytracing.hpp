#pragma once

#include <AgilitySDK/d3d12.h>
#include <Graphics/ShaderCompiler.hpp>

namespace lde
{
	class D3D12Device;
	class Model;
	class SceneCamera;
	class Scene;
	class Skybox;

	struct GeometryInfo
	{
		uint32 VertexOffset;
		uint32 IndexOffset;
	};

	class D3D12RaytracingBLAS
	{
	public:
		D3D12RaytracingBLAS() = default;

		Ref<ID3D12Resource> ScratchBuffer;
		Ref<ID3D12Resource> ResultBuffer;
		uint64 ScratchSize = 0;
		uint64 ResultSize = 0;
		
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

		std::vector<Instance> Instances;

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
		TableRecord(void* pIdentifier, uint32 Size);
		TableRecord(void* pIdentifier, uint32 Size, void* pLocalRootArgs, uint32 ArgsSize);

		void CopyTo(void* pDestination);

		struct Identifier
		{
			void* pData = nullptr;
			uint32 Size = 0;
		};

		Identifier m_Identifier;
		Identifier m_LocalRootArgs;

		// For using more then one record in a ShaderTable
		// requires aligment for largest record size
		uint32 TotalSize = 0;

	};

	class ShaderTable
	{
	public:
		ShaderTable() {}
		~ShaderTable();

		void Create(ID3D12Device8* pDevice, uint32 NumShaderRecord, uint32 ShaderRecordSize, const std::string& DebugName = "");

		void AddRecord(TableRecord& Record);

		void SetStride(uint32 Stride);
		void CheckAlignment();

		void Release();

		inline uint32 GetRecordsCount() { return static_cast<uint32>(m_Records.size()); }
		inline uint32 Stride() { return m_Stride; }

		inline uint32 GetShaderRecordSize() { return m_ShaderRecordSize; }
		inline ID3D12Resource* GetStorage() { return m_Storage.Get(); }
		inline const D3D12_GPU_VIRTUAL_ADDRESS GetAddressOf() const { return m_Storage.Get()->GetGPUVirtualAddress(); }

	private:
		Ref<ID3D12Resource> m_Storage;
		uint8* m_MappedData = nullptr;
		uint32 m_ShaderRecordSize = 0;
		uint32 m_Stride = 0;

		std::vector<TableRecord> m_Records;

	};

	// TODO:
	class D3D12ShaderTable
	{
	public:
		D3D12ShaderTable(D3D12Device* pDevice);
		~D3D12ShaderTable();

		void Create();

	private:
		Ref<ID3D12Resource> m_Storage;

	};

	class D3D12Raytracing
	{
	public:
		D3D12Raytracing(D3D12Device* pDevice, SceneCamera* pCamera);
		~D3D12Raytracing();

		void Release();

		void DispatchRaytrace();

		void CreateTLAS();
		void AddBLAS(Model* pModel);

		void BuildShaders();
		void CreateRootSignature();
		void CreateSceneUAV();
		void CreateStateObject();
		void BuildShaderTable(Scene* pScene);

		D3D12RaytracingTLAS TLAS;
		// Output resource; UAV.
		D3D12Texture* m_SceneOutput;
	private:
		D3D12Device* m_Device = nullptr; // Parent Device
		SceneCamera* m_Camera = nullptr;

		Ref<ID3D12StateObject>				m_StateObject;
		Ref<ID3D12StateObjectProperties>	m_StateObjectProperties;

		struct 
		{
			Shader* RayGen;
			Shader* ClosestHit;
			Shader* Miss;
		} Shaders;
		
		// Local Root Signatures are not needed.
		struct 
		{
			D3D12RootSignature* Global;
			//D3D12RootSignature* ClosestHit;
			//D3D12RootSignature* Miss;
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

	// TODO:
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

} // namespace lde

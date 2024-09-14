#ifndef AMPLIFICATION_HLSL
#define AMPLIFICATION_HLSL

#define GROUP_SIZE 32

struct Vertex
{
	float3 Position;
	float2 TexCoords;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
};

struct PerObject
{
	row_major float4x4 World;
	row_major float4x4 WVP;
	uint DrawMeshlets;
};

ConstantBuffer<PerObject> PerObjectData : register(b0, space0);

struct MSInput
{
	uint VerticesIndex;
	uint MeshletsIndex;
	
};

struct VertexOutput
{
	float4 Position : SV_Position;
	float4 WorldPosition : WORLD_POSITION;
	float2 TexCoords : TEXCOORDS;
	float3 Normal : NORMAL;
	float MeshletIndex : COLOR0;
};

struct Meshlet
{
	uint VertCount;
	uint VertOffset;
	uint PrimCount;
	uint PrimOffset;
};

struct MeshInfo
{
	uint IndexBytes;
	uint MeshletOffset;
};

ConstantBuffer<MeshInfo> gMeshInfo : register(b1);

StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
ByteAddressBuffer UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);

[NumThreads(GROUP_SIZE, 1, 1)]
void ASMain(in uint GroupIndex	: SV_GroupIndex,
			in uint GroupID		: SV_GroupID)
{

}

#endif // AMPLIFICATION_HLSL

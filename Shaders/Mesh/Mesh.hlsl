#ifndef MESH_HLSL
#define MESH_HLSL

// https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12MeshShaders/src/MeshletRender/MeshletMS.hlsl

#define MAX_MESHLET_SIZE		126
#define MAX_MESHLET_TRIANGLES	64

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
	float4	Position		: SV_Position;
	float4	WorldPosition	: WORLD_POSITION;
	float2	TexCoords		: TEXCOORDS;
	float3	Normal			: NORMAL;
	float	MeshletIndex	: COLOR0;
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

VertexOutput GetVertexAttributes(uint meshletIndex, uint vertexIndex)
{
	Vertex v = Vertices[vertexIndex];

	VertexOutput vout;
	vout.Position = mul(float4(v.Position, 1), PerObjectData.WVP);
	vout.Normal = mul(float4(v.Normal, 0), PerObjectData.World).xyz;
	vout.MeshletIndex = meshletIndex;

	return vout;
}

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
	return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 GetPrimitive(Meshlet m, uint index)
{
	return UnpackPrimitive(PrimitiveIndices[m.PrimOffset + index]);
}

uint GetVertexIndex(Meshlet m, uint localIndex)
{
	localIndex = m.VertOffset + localIndex;

	if (gMeshInfo.IndexBytes == 4) // 32-bit Vertex Indices
	{
		return UniqueVertexIndices.Load(localIndex * 4);
	}
	else // 16-bit Vertex Indices
	{
        // Byte address must be 4-byte aligned.
		uint wordOffset = (localIndex & 0x1);
		uint byteOffset = (localIndex / 2) * 4;

        // Grab the pair of 16-bit indices, shift & mask off proper 16-bits.
		uint indexPair = UniqueVertexIndices.Load(byteOffset);
		uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

		return index;
	}
}

[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void MSMain(
	in uint GroupThreadId : SV_GroupIndex,
	in uint GroupId : SV_GroupID,
	out indices uint3 Indices[MAX_MESHLET_SIZE],
	out vertices VertexOutput Vertices[MAX_MESHLET_SIZE]
)
{
	Meshlet m = Meshlets[gMeshInfo.MeshletOffset + GroupId];
	
	SetMeshOutputCounts(m.VertCount, m.PrimCount);

	if (GroupThreadId < m.PrimCount)
	{
		Indices[GroupThreadId] = GetPrimitive(m, GroupThreadId);
	}

	if (GroupThreadId < m.VertCount)
	{
		uint vertexIndex = GetVertexIndex(m, GroupThreadId);
		Vertices[GroupThreadId] = GetVertexAttributes(GroupThreadId, vertexIndex);
	}
}

float4 PSMain() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

#endif // MESH_HLSL

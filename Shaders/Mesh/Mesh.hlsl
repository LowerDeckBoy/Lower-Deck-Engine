#ifndef MESH_HLSL
#define MESH_HLSL

// https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html

#define MAX_MESHLET_SIZE		126
#define MAX_MESHLET_TRIANGLES	64


struct MSInput
{
	uint VerticesIndex;
	uint MeshletsIndex;
	
};

struct MSOutput
{
	float4	Position		: SV_Position;
	float4	WorldPosition	: WORLD_POSITION;
	float2	TexCoords		: TEXCOORDS;
	float3	Normal			: NORMAL;
	float	MeshletIndex	: COLOR0;
};

[NumThreads(128, 1, 1)]
[OutputTopology("triangles")]
void MSMain(
	in uint GroupThreadId : SV_GroupIndex,
	in uint GroupId : SV_GroupID,
	out vertices MSOutput Vertices[MAX_MESHLET_SIZE],
	out indices uint3 Triangles[MAX_MESHLET_TRIANGLES]
)
{

}

#endif // MESH_HLSL

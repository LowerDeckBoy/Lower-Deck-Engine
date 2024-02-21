

// Input data layout
struct VSInput
{
	float3 Position;
	float2 TexCoord;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
};

[numthreads(1, 1, 1)]
void MSmain(uint3 DTid : SV_DispatchThreadID)
{
}
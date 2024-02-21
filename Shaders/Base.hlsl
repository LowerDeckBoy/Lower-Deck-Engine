#ifndef BASE_HLSL
#define BASE_HLSL


cbuffer cbPerObject : register(b0, space0)
{
	row_major float4x4 WVP;
	row_major float4x4 World;
};

struct Vertex
{
	uint VertexIndex;
	uint VertexOffset;
};

ConstantBuffer<Vertex> vertexBuffer : register(b1, space0);

struct VSInput
{
	float3 Position;
	float2 TexCoord;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WORLD_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3x3 TBN : TBN;
	//float3 Tangent			: TANGENT;
	//float3 Bitangent		: BITANGENT;
};

VSOutput VSmain(uint VertexID : SV_VertexID)
{
	StructuredBuffer<VSInput> buffer = ResourceDescriptorHeap[vertexBuffer.VertexIndex];
	VSInput vertex = buffer.Load((vertexBuffer.VertexOffset + VertexID));
	
	VSOutput output = (VSOutput) 0;
	output.Position = mul(WVP, float4(vertex.Position, 1.0f));
	output.WorldPosition = mul(World, float4(vertex.Position, 1.0f));
	output.TexCoord = vertex.TexCoord;
	output.Normal = normalize(mul((float3x3) World, vertex.Normal));

	float3x3 TBN = float3x3(vertex.Tangent, vertex.Bitangent, output.Normal);
	output.TBN = mul((float3x3) World, transpose(TBN));
	
	return output;
}

SamplerState texSampler : register(s0, space0);

float4 PSmain(VSOutput pin) : SV_TARGET
{
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
	return float4(pin.Normal, 1.0f);
}



#endif // BASE_HLSL

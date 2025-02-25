#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

#include "../Material.hlsli"

cbuffer cbPerObject : register(b0, space0)
{
	row_major float4x4 WVP;
	row_major float4x4 World;
};

struct Vertex
{
	uint VertexIndex;
};

ConstantBuffer<Vertex> vertexBuffer : register(b1, space0);
ConstantBuffer<Material> material   : register(b2, space0);

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
	float4 Position			: SV_POSITION;
	float4 WorldPosition	: WORLD_POSITION;
	float2 TexCoord			: TEXCOORD;
	float3 Normal			: NORMAL;
	float3x3 TBN			: TBN;
};

float3 GetBitangent(float4 WorldPos, float2 UV)
{
	float3 pos_dx = ddx(WorldPos.xyz);
	float3 pos_dy = ddy(WorldPos.xyz);
	float3 tex_dx = ddx(float3(UV, 0.0));
	float3 tex_dy = ddy(float3(UV, 0.0));
	return cross(pos_dx, pos_dy);
}

// Load Vertex for current SV_VertexID.
VSInput LoadVertex(uint Location)
{
	StructuredBuffer<VSInput> buffer = ResourceDescriptorHeap[vertexBuffer.VertexIndex];
	VSInput vertex = buffer.Load(Location);
	
	return vertex;
}

VSOutput VSmain(uint VertexID : SV_VertexID)
{
	VSInput vertex = LoadVertex(VertexID);
	
	VSOutput output = (VSOutput) 0;
	output.Position			= mul(WVP, float4(vertex.Position, 1.0f));
	output.WorldPosition	= mul(World, float4(vertex.Position, 1.0f));
	output.TexCoord			= vertex.TexCoord;
	output.Normal			= normalize(mul((float3x3) World, vertex.Normal));
	
	float3x3 TBN = float3x3(vertex.Tangent, vertex.Bitangent, vertex.Normal);
	output.TBN = mul((float3x3)World, transpose(TBN));

	return output;
}

SamplerState texSampler : register(s0, space0);

struct GBufferOutput
{
	float4 Depth			: SV_Target0;
	float4 BaseColor		: SV_Target1;
	float4 TexCoords		: SV_Target2;
	float4 Normal			: SV_Target3;
	float4 MetalRoughness	: SV_Target4;
	float4 Emissive			: SV_Target5;
	float4 WorldPosition	: SV_Target6;
};

GBufferOutput PSmain(VSOutput pin)
{
	GBufferOutput output = (GBufferOutput) 0;

	output.TexCoords = float4(pin.TexCoord, 0.0f, 0.0f);
	
	const float z = 1.0f - (pin.Position.z / pin.Position.w);
	output.Depth = float4(z, z, z, 1.0f);
		
	output.WorldPosition = pin.WorldPosition;
	
	output.BaseColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (material.BaseColorIndex > INVALID_INDEX)
	{
		Texture2D<float4> texture = ResourceDescriptorHeap[material.BaseColorIndex];
		output.BaseColor = material.BaseColorFactor * texture.Sample(texSampler, pin.TexCoord);
		
		if (output.BaseColor.a < material.AlphaCutoff)
		{
			discard;
		}
	}
	
	// Load and transform Normal texture
	output.Normal = float4(pin.Normal, 1.0f);
	if (material.NormalIndex > INVALID_INDEX)
	{
		Texture2D<float4> normalTexture = ResourceDescriptorHeap[material.NormalIndex];
		float4 normalMap = normalize(2.0f * normalTexture.Sample(texSampler, pin.TexCoord) - float4(1.0f, 1.0f, 1.0f, 1.0f));
		output.Normal = float4(normalize(mul(pin.TBN, normalMap.xyz)), normalMap.w);
	
	}
	
	// Load MetalRoughness texture
	output.MetalRoughness = float4(0.0f, material.RoughnessFactor, material.MetallicFactor, 1.0f);
	if (material.MetalRoughnessIndex > INVALID_INDEX)
	{
		Texture2D<float4> metalRoughnessTex = ResourceDescriptorHeap[material.MetalRoughnessIndex];
		float4 metallic = metalRoughnessTex.Sample(texSampler, pin.TexCoord);
		metallic.b *= material.MetallicFactor;
		metallic.g *= material.RoughnessFactor;
		output.MetalRoughness = metallic;
	}
	
	// Load Emissive texture.
	output.Emissive = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (material.EmissiveIndex > INVALID_INDEX)
	{
		Texture2D<float4> emissiveTex = ResourceDescriptorHeap[material.EmissiveIndex];
		output.Emissive = emissiveTex.Sample(texSampler, pin.TexCoord) * material.EmissiveFactor;

	}

	return output;
}

#endif // GBUFFER_HLSL

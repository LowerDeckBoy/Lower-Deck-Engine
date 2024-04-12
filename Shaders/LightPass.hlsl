#ifndef LIGHTPASS_HLSL
#define LIGHTPASS_HLSL

#include "DeferredCommon.hlsli"

struct cbSceneData
{
	float4 CameraPosition;
	float4x4 View;
	row_major float4x4 InvView;
	float4x4 Projection;
	row_major float4x4 InvProjection;
	uint Width;
	uint Height;
	float zNear;
	float zFar;
};

struct cbDirLight
{
	//float4 Position;
	float4 Direction;
	float4 Ambient;
	float4 Diffuse;
};

struct GBuffers
{
	int DepthIndex;
	int BaseColorIndex;
	int TexCoordsIndex;
	int NormalIndex;
	int MetalRoughnessIndex;
	int EmissiveIndex;
	int WorldPositionIndex;
};

ConstantBuffer<cbSceneData> SceneData		: register(b0, space0);
ConstantBuffer<cbDirLight>  DirLight		: register(b1, space0);
ConstantBuffer<GBuffers>	GBufferIndices	: register(b2, space0);

ScreenQuadOutput VSmain(uint VertexID : SV_VertexID)
{
	/*
	const float4 vertices[4] =
	{
		float4(-1.0f, +1.0f, 0.0f, 1.0f),
		float4(+1.0f, +1.0f, 0.0f, 1.0f),
		float4(+1.0f, -1.0f, 0.0f, 1.0f),
		float4(-1.0f, -1.0f, 0.0f, 1.0f)
	};
	
	const float2 texcoords[4] =
	{
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 1.0f)
	};
	
	output.Position = vertices[VertexID];
	output.TexCoord = texcoords[VertexID];
	*/
	
	ScreenQuadOutput output = (ScreenQuadOutput) 0;
	output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	output.Position = float4(output.TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
	output.Position.y *= -1.0f;
	return output;
}

SamplerState texSampler : register(s0, space0);
SamplerState spBRDFSampler : register(s1, space0);

float4 PSmain(ScreenQuadOutput pin) : SV_TARGET
{
	//float2 position = pin.Position.xy;
	float2 position = pin.TexCoord;
	
	// All textures that come from GBuffer
	Texture2D<float4> texDepth				= ResourceDescriptorHeap[GBufferIndices.DepthIndex];
	Texture2D<float4> texBaseColor			= ResourceDescriptorHeap[GBufferIndices.BaseColorIndex];
	Texture2D<float4> texNormal				= ResourceDescriptorHeap[GBufferIndices.NormalIndex];
	Texture2D<float4> texMetalRoughness		= ResourceDescriptorHeap[GBufferIndices.MetalRoughnessIndex];
	Texture2D<float4> texEmissive			= ResourceDescriptorHeap[GBufferIndices.EmissiveIndex];
	Texture2D<float4> texWorldPosition		= ResourceDescriptorHeap[GBufferIndices.WorldPositionIndex];
	
	float depth			= texDepth.Load(int3(position, 0)).r;
	float4 baseColor	= pow(texBaseColor.Sample(texSampler, position), 2.2f);
	float4 normal		= normalize(texNormal.Sample(texSampler, position));
	float metalness		= texMetalRoughness.Sample(texSampler, position).b;
	float roughness		= texMetalRoughness.Sample(texSampler, position).g;
	float4 positions	= texWorldPosition.Sample(texSampler, position);
	
	float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
	float3 output = ambient;
	
	float3 N = normal.rgb;
	float3 V = normalize(SceneData.CameraPosition.xyz - positions.rgb);
	
	float NdotV = saturate(max(dot(N, V), 0.0001f));
    
	float3 F0 = lerp(0.04f, baseColor.rgb, metalness);
	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	float3 L = normalize(DirLight.Direction.xyz - positions.xyz);
	float3 H = normalize(DirLight.Direction.xyz - L);

	float distance = length(DirLight.Direction.xyz - positions.xyz);
	float attenuation = 1.0f / (distance * distance);
	float3 radiance = DirLight.Diffuse.rgb * (attenuation * DirLight.Direction.w);
        
	float NdotH = max(dot(N, H), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
	//float NdotL = max(dot(N, DirLight.Direction.xyz), 0.0f);
	
    // Cook-Torrance BRDF
	float NDF = GetDistributionGGX(N, H, roughness);
	float G = GetGeometrySmith(N, V, L, roughness);
	float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);

	float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - F, float3(0.0f, 0.0f, 0.0f), metalness);
        
	float3 numerator = NDF * G * F;
	float denominator = 4.0f * NdotV * NdotL;
	float3 specular = numerator / max(denominator, Epsilon);
        
	Lo += ((kD * baseColor.rgb / PI) + specular) * radiance  * NdotL;
	
	//float NdotL = max(dot(N, DirLight.Direction.xyz), 0.0f);
	float3 light = saturate(NdotL * DirLight.Diffuse.rgb * (baseColor.rgb));
	output *= DirLight.Ambient.rgb + ambient + Lo;
	//output += light;
	
	output = output / (output + float3(1.0f, 1.0f, 1.0f));
	output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);
	
	output += saturate(dot(DirLight.Direction.xyz, N) * DirLight.Diffuse.rgb * baseColor.rgb);
	//output += light;
	
	return float4(output, 1.0f); //  + light
}

#endif // LIGHTPASS_HLSL

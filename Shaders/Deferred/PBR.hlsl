#ifndef PBR_HLSL
#define PBR_HLSL

#include "DeferredCommon.hlsli"
#include "../Camera.hlsli"
#include "../Lights.hlsli"

struct LightsData
{
	DirectionalLight Directional;
	PointLight Light[4];
	//float4 padding[11];
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

struct IBLTextures
{
	int IrradianceIndex;
	int SpecularIndex;
	int SpecularBRDFIndex;
};

ConstantBuffer<CameraData> Camera : register(b0, space0);
ConstantBuffer<LightsData> Lights : register(b1, space0);
ConstantBuffer<GBuffers> GBufferIndices : register(b2, space0);
ConstantBuffer<IBLTextures> IBL : register(b3, space0);

ScreenQuadOutput VSmain(uint VertexID : SV_VertexID)
{
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
	const float2 position = pin.Position.xy;

	// Textures that come from GBuffer
	Texture2D<float4> texBaseColor		= ResourceDescriptorHeap[GBufferIndices.BaseColorIndex];
	Texture2D<float4> texNormal			= ResourceDescriptorHeap[GBufferIndices.NormalIndex];
	Texture2D<float4> texMetalRoughness = ResourceDescriptorHeap[GBufferIndices.MetalRoughnessIndex];
	Texture2D<float4> texEmissive		= ResourceDescriptorHeap[GBufferIndices.EmissiveIndex];
	Texture2D<float4> texWorldPosition	= ResourceDescriptorHeap[GBufferIndices.WorldPositionIndex];

	float4	baseColor		= pow(texBaseColor.Load(int3(position, 0)), 2.2f);
	float4	normal			= texNormal.Load(int3(position, 0));
	float	metalness		= texMetalRoughness.Load(int3(position, 0)).b;
	float	roughness		= texMetalRoughness.Load(int3(position, 0)).g;
	float3	worldPosition	= texWorldPosition.Load(int3(position, 0)).xyz;

	float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
	float3 output = ambient;
	
	float3 N = normal.rgb;
	float3 V = normalize(Camera.Position.xyz - worldPosition);
	
	float NdotV = max(dot(N, V), 0.0f);
	
	float3 F0 = lerp(Fdielectric, baseColor.rgb, float3(metalness, metalness, metalness));
	
	float3 F = GetFresnelSchlick(NdotV, F0);
	float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - F, float3(0.0f, 0.0f, 0.0f), float3(metalness, metalness, metalness));
	kD *= (1.0f - metalness);
	
	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < 4; ++i)
	{
		float3 L = normalize(Lights.Light[i].Position.xyz - worldPosition);
		float3 H = normalize(V + L);
	
		float NdotL = max(dot(N, L), 0.0f);
			
		float distance = length(Lights.Light[i].Position.xyz - worldPosition);
		float attenuation = 1.0f / (distance * distance + 1.0f);
		float3 radiance = Lights.Light[i].Ambient.rgb * (attenuation * Lights.Light[i].Range);

		// Cook-Torrance BRDF
		float NDF = GetDistributionGGX(N, H, roughness);
		float G = GetGeometrySmith(N, V, L, roughness);

		float3	numerator = NDF * G * F;
		float	denominator = 4.0f * NdotV * NdotL;
		float3	specular = numerator / max(denominator, Epsilon);
		
		Lo += Lights.Light[i].Visibility * ((kD * (baseColor.rgb / PI)) + specular) * radiance * NdotL;
	}
	
	float3 directional = float3(0.0f, 0.0f, 0.0f);
	{
		//float3 L = normalize(Lights.Directional.Direction.xyz - worldPosition);
		float3 L = normalize(-Lights.Directional.Direction.xyz);
		float3 H = normalize(L + V);
	
		float NdotL = max(dot(N, L), 0.0f);

		float3 radiance = Lights.Directional.Ambient.rgb;
		
		// Cook-Torrance BRDF
		float NDF = GetDistributionGGX(N, H, roughness);
		float G = GetGeometrySmith(N, V, L, roughness);
		
		float3	numerator = NDF * G * F;
		float	denominator = 4.0f * NdotV * NdotL;
		float3	specular = numerator / max(denominator, Epsilon);
		
		directional += ((kD * (baseColor.rgb / PI)) + specular) * radiance * NdotL;

	}
	
	// Reflection vector
	float3 Lr = normalize(reflect(-V, N));
	
	float3 ambientLighting = float3(0.0f, 0.0f, 0.0f);
	{
		TextureCube<float4> texIrradiance	= ResourceDescriptorHeap[IBL.IrradianceIndex];
		TextureCube<float4> texSpecular		= ResourceDescriptorHeap[IBL.SpecularIndex];
		Texture2D<float4>	texSpecularBRDF = ResourceDescriptorHeap[IBL.SpecularBRDFIndex];

		float3 irradiance = texIrradiance.Sample(texSampler, N).rgb;
		float3 diffuseIBL = (kD * (baseColor.rgb / PI) * irradiance);
		
		uint width, height, mipLevels;
		texSpecular.GetDimensions(0, width, height, mipLevels);
		
		const float lod = roughness * mipLevels;
		float3 specular = texSpecular.SampleLevel(texSampler, Lr, lod).rgb;
		float2 specularBRDF = texSpecularBRDF.Sample(spBRDFSampler, float2(NdotV, roughness)).rg;
		float3 specularIBL = specular * (F0 * specularBRDF.x + specularBRDF.y);

		ambientLighting = diffuseIBL + specularIBL;
	}
	
	output += texEmissive.Sample(texSampler, pin.TexCoord).rgb + Lo + directional + ambientLighting;
	
	output = output / (output + float3(1.0f, 1.0f, 1.0f));
	output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);
	
	return float4(output, 1.0f);
}

#endif // PBR_HLSL

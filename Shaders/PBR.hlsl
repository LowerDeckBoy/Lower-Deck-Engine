#ifndef PBR_HLSL
#define PBR_HLSL

#include "DeferredCommon.hlsli"
#include "Camera.hlsli"


struct DirectionalLight
{
	float4 Direction;
	float4 Ambient;
	float4 Diffuse;
};

struct PointLight
{
	float4 Position;
	float4 Ambient;
	float Range;
	float3 padding;
};

struct LightsData
{
	DirectionalLight Directional;
	PointLight Light[4];
	float4 padding[11];
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
	int SkyboxIndex;
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

/*
float3 CalcPointLight(PointLight Light, float4 BaseColor)
{
	float3 L = normalize(Light.Position.xyz - positions.xyz);
	float3 H = normalize(L + V);
	
	float NdotH = max(dot(N, H), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
		
	float distance = length(Lights.Light[i].Position.xyz - positions.xyz);
	float attenuation = 1.0f / (distance * distance);
	float3 radiance = Lights.Light[i].Ambient.rgb * (attenuation * Lights.Light[i].Range);
	
        // Cook-Torrance BRDF
	float NDF = GetDistributionGGX(N, H, roughness);
	float G = GetGeometrySmith(N, V, L, roughness);
	float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);
	
	float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - F, float3(0.0f, 0.0f, 0.0f), metalness);
		//kD *= (1.0f - metalness);
        
	float3 numerator = NDF * G * F;
	float  denominator = 4.0f * NdotV * NdotL;
	float3 specular = numerator / max(denominator, Epsilon);
	
	float3 output = ((kD * (BaseColor.rgb / PI)) + specular) * radiance * NdotL;
	
	return float3()
}*/

SamplerState texSampler : register(s0, space0);
SamplerState spBRDFSampler : register(s1, space0);

float4 PSmain(ScreenQuadOutput pin) : SV_TARGET
{
	//float2 position = pin.Position.xy;
	float2 position = pin.TexCoord;
	
	// All textures that come from GBuffer
	Texture2D<float4> texDepth			= ResourceDescriptorHeap[GBufferIndices.DepthIndex];
	Texture2D<float4> texBaseColor		= ResourceDescriptorHeap[GBufferIndices.BaseColorIndex];
	Texture2D<float4> texNormal			= ResourceDescriptorHeap[GBufferIndices.NormalIndex];
	Texture2D<float4> texMetalRoughness = ResourceDescriptorHeap[GBufferIndices.MetalRoughnessIndex];
	Texture2D<float4> texEmissive		= ResourceDescriptorHeap[GBufferIndices.EmissiveIndex];
	Texture2D<float4> texWorldPosition	= ResourceDescriptorHeap[GBufferIndices.WorldPositionIndex];
	
	float depth			= texDepth.Load(int3(position, 0)).r;
	float4 baseColor	= pow(texBaseColor.Sample(texSampler, position), 2.2f); // Gamma correction
	float4 normal		= normalize(texNormal.Sample(texSampler, position));
	float metalness		= texMetalRoughness.Sample(texSampler, position).b;
	float roughness		= texMetalRoughness.Sample(texSampler, position).g;
	float4 positions	= texWorldPosition.Sample(texSampler, position);
	
	float3 ambient = float3(0.03f, 0.03f, 0.03f) * baseColor.rgb * float3(1.0f, 1.0f, 1.0f);
	float3 output = float3(0.0f, 0.0f, 0.0f);
	
	float3 N = normal.rgb;
	float3 V = normalize(Camera.Position.xyz - positions.rgb);
	
	float NdotV = max(dot(N, V), 0.0001f);
    
	float3 F0 = lerp(0.04f, baseColor.rgb, metalness);
	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < 4; ++i)
	{
		float3 L = normalize(Lights.Light[i].Position.xyz - positions.xyz);
		float3 H = normalize(L + V);
	
		float NdotL = max(dot(N, L), 0.0f);
		
		float  distance = length(Lights.Light[i].Position.xyz - positions.xyz);
		float  attenuation = 1.0f / (distance * distance);
		float3 radiance = Lights.Light[i].Ambient.rgb * (attenuation * Lights.Light[i].Range);
	
        // Cook-Torrance BRDF
		float NDF = GetDistributionGGX(N, H, roughness);
		float G = GetGeometrySmith(N, V, L, roughness);
		float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);
	
		float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - F, float3(0.0f, 0.0f, 0.0f), metalness);
		kD *= (1.0f - metalness);
        
		float3 numerator = NDF * G * F;
		float  denominator = 4.0f * NdotV * NdotL;
		float3 specular = numerator / max(denominator, Epsilon);
        
		Lo += ((kD * (baseColor.rgb /* / PI*/)) + specular) * radiance * NdotL;
	}
	
	float3 directional = float3(0.0f, 0.0f, 0.0f);
	{
		//float3 L = -normalize(Lights.Directional.Direction.xyz - positions.xyz);
		float3 L = -Lights.Directional.Direction.xyz;
		float3 H = normalize(L + V);
	
		//float NdotH = max(dot(N, H), 0.0f);
		float NdotL = max(dot(N, L), 0.0f);
	
		float NDF = GetDistributionGGX(N, H, roughness);
		float G = GetGeometrySmith(N, V, L, roughness);
		float3 F = GetFresnelSchlick(max(dot(H, V), 0.0f), F0);
	
		float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - F, float3(0.0f, 0.0f, 0.0f), metalness);
		kD *= (1.0f - metalness);
		
		float3 numerator = NDF * G * F;
		float denominator = 4.0f * NdotV * NdotL;
		float3 specular = numerator / max(denominator, Epsilon);
	
		directional += ((kD * baseColor.rgb) + specular) * NdotL * Lights.Directional.Ambient.rgb;
	}
	
	output = ambient;
	
	// Reflection vector
	float3 Lr = reflect(-V, N);
	
	float3 ambientLighting = float3(0.0f, 0.0f, 0.0f);
	{
		TextureCube<float4> texIrradiance	= ResourceDescriptorHeap[IBL.IrradianceIndex];
		TextureCube<float4> texSpecular		= ResourceDescriptorHeap[IBL.SpecularIndex];
		Texture2D<float2>   texSpecularBRDF	= ResourceDescriptorHeap[IBL.SpecularBRDFIndex];
	
		float3 F  = GetFresnelSchlick(NdotV, F0);
		float3 kS = F;
		float3 kD = lerp(float3(1.0f, 1.0f, 1.0f) - kS, float3(0.0f, 0.0f, 0.0f), metalness);
		kD *= (1.0f - metalness);
		
		float3 irradiance = texIrradiance.Sample(texSampler, N).rgb;
		float3 diffuseIBL = (kD * baseColor.rgb * irradiance);
		
		uint width, height, levels;
		texSpecular.GetDimensions(0, width, height, levels);
		
		float3 specular = texSpecular.SampleLevel(texSampler, Lr, roughness * (levels - 1)).rgb;
		//float2 specularBRDF = texSpecularBRDF.Sample(spBRDFSampler, float2(min(NdotV, 0.999f), roughness)).rg;
		float2 specularBRDF = texSpecularBRDF.Sample(spBRDFSampler, float2(NdotV, roughness)).rg;
		float3 specularIBL	= (F0 * specularBRDF.x + specularBRDF.y) * specular;

		ambientLighting = (diffuseIBL + specularIBL);
	}
	
	output += texEmissive.Sample(texSampler, pin.TexCoord).rgb + Lo + directional + ambientLighting;
	
	output = output / (output + float3(1.0f, 1.0f, 1.0f));
	output = lerp(output, pow(output, 1.0f / 2.2f), 0.4f);
	
	return float4(output, 1.0f);
}

#endif // PBR_HLSL

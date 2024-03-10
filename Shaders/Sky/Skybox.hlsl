#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};

cbuffer cbPerObject : register(b0)
{
	row_major float4x4 WVP;
	row_major float4x4 World;
};

struct TextureIndices
{
	int SkyboxTexture;
};

ConstantBuffer<TextureIndices> Textures : register(b1, space0);

VS_OUTPUT VSmain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	float3 vertices[8] =
	{
		float3(-1.0f, +1.0f, +1.0f),
	    float3(+1.0f, +1.0f, +1.0f),
	    float3(+1.0f, -1.0f, +1.0f),
	    float3(-1.0f, -1.0f, +1.0f),
	    float3(+1.0f, +1.0f, -1.0f),
	    float3(-1.0f, +1.0f, -1.0f),
	    float3(-1.0f, -1.0f, -1.0f),
		float3(+1.0f, -1.0f, -1.0f)
	};
	
	output.Position = mul(WVP, float4(vertices[VertexID], 1.0f)).xyww;
	output.TexCoord = normalize(vertices[VertexID]);
	
	return output;
}

SamplerState texSampler : register(s0, space0);

float4 PSmain(VS_OUTPUT pin) : SV_TARGET
{
	float3 skyTexture = float3(0.0f, 1.0f, 0.0f);
	if (Textures.SkyboxTexture != -1)
	{
		TextureCube<float4> skyboxTex = ResourceDescriptorHeap[Textures.SkyboxTexture];
		skyTexture = skyboxTex.Sample(texSampler, pin.TexCoord).rgb;
	}
	
	return float4(pin.TexCoord.rgb, 1.0f);
}

#endif // SKYBOX_HLSL

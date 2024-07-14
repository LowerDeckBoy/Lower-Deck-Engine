#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

// ==================================================
// Shaders/Sky/Skybox.hlsl
// Samples Sky TextureCube from given index.
// ==================================================

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

static const float3 Vertices[8] =
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

VS_OUTPUT VSmain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = mul(WVP, float4(Vertices[VertexID], 1.0f)).xyww;
	output.TexCoord = Vertices[VertexID];

	return output;
}

SamplerState texSampler : register(s0, space0);

static const float2 invAtan = float2(0.1591f, 0.3183f);

float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(-v.y));
	uv *= invAtan;
	uv += 0.5f;
	return uv;
}

float4 PSmain(VS_OUTPUT pin) : SV_TARGET
{
	float3 skyTexture = float3(0.0f, 0.0f, 0.0f);
	if (Textures.SkyboxTexture != -1)
	{
		// Note: normalize TexCoords in pixel shader. Normalizing only in Vertex shader will not do.
		//float2 uv = SampleSphericalMap(normalize(pin.TexCoord));
		//Texture2D<float4> skyboxTex = ResourceDescriptorHeap[Textures.SkyboxTexture];
		//skyTexture = skyboxTex.Sample(texSampler, uv, 0.0f).rgb;
		
		TextureCube<float4> skyboxTex = ResourceDescriptorHeap[Textures.SkyboxTexture];
		skyTexture = skyboxTex.Sample(texSampler, pin.TexCoord).rgb;
		// Gamma correction
		skyTexture = skyTexture / (skyTexture + float3(1.0f, 1.0f, 1.0f));
		skyTexture = lerp(skyTexture, pow(skyTexture, 1.0f / 2.2f), 0.4f);
	}

	return float4(skyTexture, 1.0f);
}

#endif // SKYBOX_HLSL

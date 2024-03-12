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

struct VS_INPUT
{
	float3 Position;
};

struct Vertex
{
	uint Index;
	uint Offset;
};

ConstantBuffer<TextureIndices> Textures : register(b1, space0);
ConstantBuffer<Vertex> VertexBuffer : register(b2, space0);

VS_OUTPUT VSmain(uint VertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	
	//float3 vertices[8] =
	//{
	//	float3(-1.0f, +1.0f, +1.0f),
	//	float3(+1.0f, +1.0f, +1.0f),
	//	float3(+1.0f, -1.0f, +1.0f),
	//	float3(-1.0f, -1.0f, +1.0f),
	//	float3(+1.0f, +1.0f, -1.0f),
	//	float3(-1.0f, +1.0f, -1.0f),
	//	float3(-1.0f, -1.0f, -1.0f),
	//	float3(+1.0f, -1.0f, -1.0f)
	//};
	
	//float3 vertices[8] =
	//{
	//	float3(-1.0f, -1.0f, -1.0f),
	//    float3(-1.0f, +1.0f, -1.0f),
	//    float3(+1.0f, +1.0f, -1.0f),
	//    float3(+1.0f, -1.0f, -1.0f),
	//    float3(-1.0f, -1.0f, +1.0f),
	//    float3(-1.0f, +1.0f, +1.0f),
	//    float3(+1.0f, +1.0f, +1.0f),
	//	float3(+1.0f, -1.0f, +1.0f)
	//};
	
	StructuredBuffer<VS_INPUT> buffer = ResourceDescriptorHeap[VertexBuffer.Index];
	VS_INPUT vertex = buffer.Load((VertexBuffer.Offset + VertexID));
	
	output.Position = mul(WVP, float4(vertex.Position, 1.0f)).xyzw;
	output.TexCoord = normalize(vertex.Position);
	//output.Position = mul(WVP, float4(vertices[VertexID], 1.0f)).xyzw;
	//output.TexCoord = normalize(vertices[VertexID]);
	
	return output;
}

SamplerState texSampler : register(s0, space0);

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), -asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

float4 PSmain(VS_OUTPUT pin) : SV_TARGET
{
	float3 skyTexture = float3(0.0f, 0.0f, 0.0f);
	if (Textures.SkyboxTexture != -1)
	{
		float2 uv = SampleSphericalMap(pin.TexCoord);
		Texture2D<float4> skyboxTex = ResourceDescriptorHeap[Textures.SkyboxTexture];
		skyTexture = skyboxTex.Sample(texSampler, uv).rgb;
		
		skyTexture = skyTexture / (skyTexture + float3(1.0f, 1.0f, 1.0f));
		skyTexture = pow(skyTexture, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
	}
	
	return float4(skyTexture, 1.0f);
}

#endif // SKYBOX_HLSL

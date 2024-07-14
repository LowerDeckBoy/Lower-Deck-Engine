#ifndef SHARED_HLSLI
#define SHARED_HLSLI

SamplerState textureSampler : register(s0, space0);

struct SceneData
{
	uint OutputIndex;
	uint TopLevelIndex;
};

ConstantBuffer<SceneData> SceneImages : register(b0, space0);

struct SceneCamera
{
	float4x4 ViewProjection;
	float4 CameraPosition;
};
ConstantBuffer<SceneCamera> Camera : register(b1, space0);

struct Vertex
{
	float3 Position;
	float2 TexCoords;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
};

struct HitPayload
{
	float4 ColorAndT;
};

struct HitInfo
{
	float4 Color;
};

float3 WorldPosition()
{
	return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

struct SkyTextures
{
	uint SkyboxIndex;
};

ConstantBuffer<SkyTextures> Sky : register(b2, space0);


struct InstanceData
{
	uint Vertex;
	uint Index;
};

StructuredBuffer<InstanceData> PerInstance;

StructuredBuffer<Vertex> VertexBuffers[] : register(t5, space0);

#endif // SHARED_HLSLI

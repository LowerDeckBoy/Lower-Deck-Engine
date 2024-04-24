#ifndef SHARED_HLSLI
#define SHARED_HLSLI

//RWTexture2D<float4>				gSceneBVH : register(u0, space0);
//RaytracingAccelerationStructure gTopLevel : register(t0, space0);

struct SceneData
{
	uint OutputIndex;
	uint TopLevelIndex;
};

ConstantBuffer<SceneData> SceneImages : register(b0, space0);

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

#endif // SHARED_HLSLI

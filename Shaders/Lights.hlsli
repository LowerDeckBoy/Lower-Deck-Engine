#ifndef LIGHTS_HLSLI
#define LIGHTS_HLSLI

// ===================================================
// Shaders/Lights.hlsli 
// Structures representing different light components.
// ===================================================

struct DirectionalLight
{
	float3 Direction;
	float padding;
	float4 Ambient;
	float4 Diffuse;
};

struct PointLight
{
	float3 Position;
	float padding;
	float4 Ambient;
	float Radius;
	float3 padding2;
};

#endif // LIGHTS_HLSLI

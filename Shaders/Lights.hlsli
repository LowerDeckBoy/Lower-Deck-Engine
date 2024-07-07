#ifndef LIGHTS_HLSLI
#define LIGHTS_HLSLI

// ===================================================
// Shaders/Lights.hlsli 
// Structures representing different light components.
// ===================================================

struct DirectionalLight
{
	float3	Direction;
	float	Visibility;
	float4	Ambient;
	float4	Diffuse;
	int		bCastShadows;
	float3	padding;
};

struct PointLight
{
	float3	Position;
	float	Visibility;
	float4	Ambient;
	float	Range;
	float3	padding;
};

// TODO:
struct SpotLight
{

};

#endif // LIGHTS_HLSLI

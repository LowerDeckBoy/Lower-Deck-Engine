#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "Shared.hlsli"
#include "../Camera.hlsli"

//ConstantBuffer<CameraData> Camera : register(b1, space0);

// Primary rays
[shader("raygeneration")]
void RayGen()
{
	HitInfo payload;
	payload.Color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dims = float2(DispatchRaysDimensions().xy);
	float2 d = ((launchIndex.xy + 0.5f) / dims.xy) * 2.f - 1.f;
	
	float2 xy = launchIndex + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;
	
	 // Y inversion
	screenPos.y = -screenPos.y;
	float aspectRatio = dims.x / dims.y;
	float4x4 viewProj = Camera.ViewProjection;
	float4 world = mul(float4(screenPos, 0, 1), viewProj);
	world.xyz /= world.w;
	float3 origin = Camera.CameraPosition.xyz;
	float3 direction = normalize(world.xyz - origin);
	
	RayDesc ray;
	ray.Origin = origin;
	ray.Direction = direction;
	// TMin -> zNear, TMax -> zFar
	ray.TMin = 0.0f;
	ray.TMax = 10000.0f;
	
	RWTexture2D<float4> outputTex = ResourceDescriptorHeap[SceneImages.OutputIndex];
	RaytracingAccelerationStructure topLevel = ResourceDescriptorHeap[SceneImages.TopLevelIndex];
	
	TraceRay(topLevel, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);
	
	outputTex[launchIndex] = float4(payload.Color.rgb, 1.f);
}

#endif // RAYGEN_HLSL

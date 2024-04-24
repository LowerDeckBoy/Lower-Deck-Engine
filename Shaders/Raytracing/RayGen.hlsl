#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "Shared.hlsli"

[shader("raygeneration")]
void RayGen()
{
	HitInfo payload;
	payload.Color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dims = float2(DispatchRaysDimensions().xy);
	float2 d = ((launchIndex.xy + 0.5f) / dims.xy) * 2.f - 1.f;
    
	RayDesc ray;
	ray.Origin = float3(d.x, -d.y, 1.0f);
	ray.Direction = float3(0.0f, 0.0f, -1.0f);
    // TMin -> zNear, TMax -> zFar
	ray.TMin = 0.0f;
	ray.TMax = 100000.0f;
	
	RWTexture2D<float4> outputTex = ResourceDescriptorHeap[SceneImages.OutputIndex];
	RaytracingAccelerationStructure topLevel = ResourceDescriptorHeap[SceneImages.TopLevelIndex];
    
	TraceRay(topLevel, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    
	outputTex[launchIndex] = float4(payload.Color.rgb, 1.f);
}

#endif // RAYGEN_HLSL

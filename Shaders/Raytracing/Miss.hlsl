#ifndef MISS_HLSL
#define MISS_HLSL

#include "Shared.hlsli"

[shader("miss")]
void Miss(inout HitInfo Payload : SV_RayPayload)
{
	uint2 launchIndex = DispatchRaysIndex().xy;
	float2 dims = float2(DispatchRaysDimensions().xy);

	uint geometryIndex = GeometryIndex();
	
	float ramp = launchIndex.y / dims.y;
	Payload.Color = float4(0.5f, 0.2f, 0.7f - 0.3f * ramp, -1.0f);
}

#endif // MISS_HLSL

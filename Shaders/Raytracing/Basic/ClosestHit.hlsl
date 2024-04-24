#ifndef CLOSESTHIT_HLSL
#define CLOSESTHIT_HLSL

#include "Shared.hlsli"

[shader("closesthit")]
void ClosestHit(inout HitPayload Payload, BuiltInTriangleIntersectionAttributes Attributes)
{
	float3 barycentrics = float3(1.f - Attributes.barycentrics.x - Attributes.barycentrics.y, Attributes.barycentrics.x, Attributes.barycentrics.y);

	uint vertId = 3 * PrimitiveIndex();
	float3 hitColor = float3(0.5f, 0.5f, 0.5f);

	Payload.ColorAndT = float4(hitColor, RayTCurrent());
}

#endif // CLOSESTHIT_HLSL

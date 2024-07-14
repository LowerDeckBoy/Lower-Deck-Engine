#ifndef CAMERA_HLSLI
#define CAMERA_HLSLI

// ==================================================
// Shaders/Camera.hlsli
// Camera data.
// ==================================================

struct CameraData
{
	float4		Position;
	float4x4	View;
	float4x4	Projection;
	row_major float4x4	InversedView;
	row_major float4x4	InversedProjection;
	uint		Width;
	uint		Height;
	float		zNear;
	float		zFar;
};

static const float3 FORWARD_VECTOR	= float3(0.0f, 0.0f, 1.0f);
static const float3 RIGHT_VECTOR	= float3(1.0f, 0.0f, 0.0f);
static const float3 UP_VECTOR		= float3(0.0f, 1.0f, 0.0f);

static void ScreenToViewParams()
{
	float4x4 InverseProjection;
	float2 ScreenDimensions;
}

// Convert clip space coordinates to view space
static float4 ClipToView(float4 clip, float4x4 InverseProj)
{
    // View space position.
	float4 view = mul(InverseProj, clip);
    // Perspective projection.
	view = view / view.w;
 
	return view;
}
 
// Convert screen space coordinates to view space.
static float4 ScreenToView(float4 screen, float2 ScreenDims, float4x4 InverseProj)
{
    // Convert to normalized texture coordinates
	float2 texCoord = screen.xy / ScreenDims;
 
    // Convert to clip space
	float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);
 
	return ClipToView(clip, InverseProj);
}

#endif // CAMERA_HLSLI

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
	float4x4	InversedView;
	float4x4	InversedProjection;
	uint		Width;
	uint		Height;
	float		zNear;
	float		zFar;
};

// https://github.com/pkurth/D3D12Renderer/blob/master/shaders/common/camera.hlsli

#endif // CAMERA_HLSLI

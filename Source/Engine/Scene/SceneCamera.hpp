#pragma once
#define _XM_SSE4_INTRINSICS_
#include <DirectXMath.h>
#include <array>

// TODO:
// https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/input/overviews/input-readings
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include "Entity.hpp"

namespace lde
{
	using namespace DirectX;
	
	class SceneCamera : public Entity
	{
		friend class Entity;
	public:
		SceneCamera() = default;
		SceneCamera(World* pWorld, float AspectRatio);
		~SceneCamera();
	
		void Initialize(World* pWorld, float AspectRatio);
	
		void Update();
	
		void SetPosition(const std::array<float, 3> NewPosition) noexcept;
	
		inline void ResetPosition() noexcept { m_Position = m_DefaultPosition; }
		inline void ResetYaw() noexcept { m_Yaw = 0.0f; }
		inline void ResetPitch() noexcept { m_Pitch = 0.0f; }
		void ResetFieldOfView() noexcept;
		void ResetCamera() noexcept;
	
		const XMMATRIX GetView() const noexcept { return XMLoadFloat4x4(&m_View); }
		const XMMATRIX GetProjection() const noexcept { return XMLoadFloat4x4(&m_Projection); }
		const XMMATRIX GetViewProjection() noexcept { return XMMatrixMultiply(XMLoadFloat4x4(&m_View), XMLoadFloat4x4(&m_Projection)); }
	
		const XMFLOAT4X4 GetViewFloats() const noexcept { return m_View; }
		const XMFLOAT4X4 GetProjectionFloats() const noexcept { return m_Projection; }
	
		const XMFLOAT3 GetPositionFloat() const noexcept { return m_Position; }
		const XMVECTOR GetPosition() const noexcept { return XMLoadFloat3(&m_Position); }
		const XMVECTOR GetTarget() const noexcept { return XMLoadFloat3(&m_Target); }
		const XMVECTOR GetUp() const noexcept { return XMLoadFloat3(&m_Up); }
	
		void OnAspectRatioChange(float NewAspectRatio) noexcept;
	
		//float GetCameraSpeed() const noexcept { return m_Camera;
		//void SetCameraSpeed(float NewSpeed) noexcept;
	
		inline void  SetZNear(float NewZ) noexcept	{ m_zNear = NewZ; }
		inline void  SetZFar(float NewZ) noexcept	{ m_zFar = NewZ; }
		inline float GetZNear() const noexcept		{ return m_zNear; }
		inline float GetZFar() const noexcept		{ return m_zFar; }

	private:
	
		XMFLOAT4X4 m_View			= XMFLOAT4X4();
		XMFLOAT4X4 m_Projection		= XMFLOAT4X4();
		XMFLOAT4X4 m_ViewProjection	= XMFLOAT4X4();
	
		XMFLOAT3 m_Position	= XMFLOAT3(0.0f, 1.0f, -10.0f);
		XMFLOAT3 m_Target	= XMFLOAT3(0.0f, 5.0f, 0.0f);
		XMFLOAT3 m_Up		= XMFLOAT3(0.0f, 1.0f, 0.0f);
	
		XMFLOAT4X4 m_RotationX		= XMFLOAT4X4();
		XMFLOAT4X4 m_RotationY		= XMFLOAT4X4();
		XMFLOAT4X4 m_RotationMatrix	= XMFLOAT4X4();
	
		XMFLOAT3 m_Forward	= XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMFLOAT3 m_Right	= XMFLOAT3(1.0f, 0.0f, 0.0f);
		XMFLOAT3 m_Upward	= XMFLOAT3(0.0f, 1.0f, 0.0f);
	
		XMFLOAT3 const m_DefaultPosition = XMFLOAT3(0.0f, 1.0f, -10.0f);
		XMFLOAT3 const m_DefaultTarget	 = XMFLOAT3(0.0f, 5.0f, 0.0f);
		XMFLOAT3 const m_DefaultUp		 = XMFLOAT3(0.0f, 1.0f, 0.0f);
	
		XMFLOAT3 const m_DefaultForward	= XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMFLOAT3 const m_DefaultRight	= XMFLOAT3(1.0f, 0.0f, 0.0f);
		XMFLOAT3 const m_DefaultUpward	= XMFLOAT3(0.0f, 1.0f, 0.0f);
	
		float m_zNear	= 0.1f;
		float m_zFar	= 500'000.0f;
	
		float m_AspectRatio = 1.0f;
		float m_FieldOfView = 45.0f;
	
	public:
		// For calling camera movement from keyboard inputs
		float MoveForwardBack	= 0.0f;
		float MoveRightLeft		= 0.0f;
		float MoveUpDown		= 0.0f;
	
		float m_Pitch	= 0.0f;
		float m_Yaw		= 0.0f;
	
		// For GUI usage
		std::array<float, 3> CameraSlider{ };
	
		inline static float CameraSpeed = 25.0f;
	
		// Inputs
	public:
		void InitializeInputs();
		void ProcessInputs(float DeltaTime);
	private:
		void ReleaseInputs();
		inline static IDirectInputDevice8* DxKeyboard{};
		inline static IDirectInputDevice8* DxMouse{};
		inline static LPDIRECTINPUT8 DxInput{};
		inline static DIMOUSESTATE DxLastMouseState{};
	};

} // namespace lde

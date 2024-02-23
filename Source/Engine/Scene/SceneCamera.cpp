#include "Components/CameraComponent.hpp"
#include "Components/Components.hpp"
#include "RHI/D3D12/D3D12Utility.hpp"
#include "SceneCamera.hpp"
#include <Platform/Window.hpp>

#pragma comment(lib, "dinput8")

namespace lde
{
	SceneCamera::SceneCamera(World* pWorld, float AspectRatio)
	{
		Initialize(pWorld, AspectRatio);
	}
	
	SceneCamera::~SceneCamera()
	{
		ReleaseInputs();
	}

	void SceneCamera::Initialize(World* pWorld, float AspectRatio)
	{
		Entity::Create(pWorld);
	
		// Defaulting positions on startup
		m_Position = m_DefaultPosition;
		m_Target = m_DefaultTarget;
		m_Up = m_DefaultUp;
	
		m_AspectRatio = AspectRatio;
		XMStoreFloat4x4(&m_View, XMMatrixLookAtLH(GetPosition(), GetTarget(), GetUp()));
		XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(m_FieldOfView, m_AspectRatio, m_zNear, m_zFar));
	
		CameraSlider = { m_Position.x, m_Position.y, m_Position.z };
	
		AddComponent<TagComponent>("Scene Camera");
		AddComponent<CameraComponent>();
	}

	void SceneCamera::Update()
	{
		// Load vectors and matrices
		XMVECTOR forward{ XMLoadFloat3(&m_Forward) };
		XMVECTOR right{ XMLoadFloat3(&m_Right) };
		XMVECTOR up{ XMLoadFloat3(&m_Up) };
		XMMATRIX rotationMatrix{ XMLoadFloat4x4(&m_RotationMatrix) };
		XMVECTOR target{ XMLoadFloat3(&m_Target) };
		//XMVECTOR position{ XMLoadFloat3(&m_Position) };
	
		auto& comp = Entity::GetComponent<CameraComponent>();
		XMVECTOR position{ XMLoadFloat3(&comp.Position) };
	
		rotationMatrix = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
		target = XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&m_DefaultForward), rotationMatrix));
	
		const XMMATRIX rotation{ XMMatrixRotationY(m_Yaw) };
	
		forward = XMVector3TransformCoord(XMLoadFloat3(&m_DefaultForward), rotation);
		right	= XMVector3TransformCoord(XMLoadFloat3(&m_DefaultRight), rotation);
		up		= XMVector3TransformCoord(XMLoadFloat3(&m_Up), rotation);
	
		position += (MoveForwardBack * forward);
		position += (MoveRightLeft * right);
		position += (MoveUpDown * up);
	
		MoveForwardBack = 0.0f;
		MoveRightLeft	= 0.0f;
		MoveUpDown		= 0.0f;
	
		target += position;
	
		XMStoreFloat4x4(&m_View, XMMatrixLookAtLH(position, target, up));
		CameraSlider = { XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position) };
	
		// Store vector and matrices
		XMStoreFloat3(&m_Forward, forward);
		XMStoreFloat3(&m_Right, right);
		XMStoreFloat3(&m_Up, up);
		XMStoreFloat4x4(&m_RotationMatrix, rotationMatrix);
		XMStoreFloat3(&m_Target, target);
		XMStoreFloat3(&m_Position, position);
	
		XMStoreFloat3(&comp.Position, position);
	
	}

	void SceneCamera::SetPosition(const std::array<float, 3> NewPosition) noexcept
	{
		m_Position = XMFLOAT3(NewPosition.at(0), NewPosition.at(1), NewPosition.at(2));
	}

	inline void SceneCamera::ResetFieldOfView() noexcept
	{
		m_FieldOfView = 45.0f;
		XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), m_AspectRatio, m_zNear, m_zFar));
	}

	void SceneCamera::ResetCamera() noexcept
	{
		auto& comp = Entity::GetComponent<CameraComponent>();
		ResetPitch();
		ResetYaw();
		ResetFieldOfView();
		ResetPosition();
	
		comp.Position	= m_DefaultPosition;
		comp.FoV		= m_FieldOfView;
		comp.Speed		= 25.0f;
	}

	void SceneCamera::OnAspectRatioChange(float NewAspectRatio) noexcept
	{
		m_AspectRatio = NewAspectRatio;
		XMStoreFloat4x4(&m_Projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FieldOfView), NewAspectRatio, m_zNear, m_zFar));
	}

	void SceneCamera::InitializeInputs()
	{
		RHI::DX_CALL(DirectInput8Create(Window::GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&DxInput), NULL));
		RHI::DX_CALL(DxInput->CreateDevice(GUID_SysKeyboard, &DxKeyboard, NULL));
		RHI::DX_CALL(DxKeyboard->SetDataFormat(&c_dfDIKeyboard));
		RHI::DX_CALL(DxKeyboard->SetCooperativeLevel(Window::GetHWnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
		RHI::DX_CALL(DxInput->CreateDevice(GUID_SysMouse, &DxMouse, NULL));
		RHI::DX_CALL(DxMouse->SetDataFormat(&c_dfDIMouse));
		RHI::DX_CALL(DxMouse->SetCooperativeLevel(Window::GetHWnd(), DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND));
	}

	void SceneCamera::ProcessInputs(float DeltaTime)
	{
		DIMOUSESTATE mouseState{};
		constexpr int keys{ 256 };
		std::array<BYTE, keys> keyboardState{};

		DxKeyboard->Acquire();
		DxMouse->Acquire();

		DxMouse->GetDeviceState(sizeof(mouseState), reinterpret_cast<LPVOID>(&mouseState));
		DxKeyboard->GetDeviceState(sizeof(keyboardState), reinterpret_cast<LPVOID>(&keyboardState));

		constexpr int state{ 0x80 };

		// ESC to exit
		if (keyboardState.at(DIK_ESCAPE) & state)
			Window::bShouldQuit = true;

		// If RMB is not held - skip mouse and keyboard camera controls
		if (!mouseState.rgbButtons[1])
		{
			Window::OnCursorShow();
			return;
		}

		Window::OnCursorHide();

		auto& comp = Entity::GetComponent<CameraComponent>();
		const float speed{ comp.Speed * static_cast<float>(DeltaTime) };
		constexpr float intensity{ 0.001f };
		constexpr float upDownIntensity{ 0.75f };

		if ((mouseState.lX != DxLastMouseState.lX) || (mouseState.lY != DxLastMouseState.lY))
		{
			m_Yaw += mouseState.lX * intensity;
			m_Pitch += mouseState.lY * intensity;
			DxLastMouseState = mouseState;
		}
		if (keyboardState.at(DIK_W) & state)
		{
			MoveForwardBack += speed;
		}
		if (keyboardState.at(DIK_S) & state)
		{
			MoveForwardBack -= speed;
		}
		if (keyboardState.at(DIK_A) & state)
		{
			MoveRightLeft -= speed;
		}
		if (keyboardState.at(DIK_D) & state)
		{
			MoveRightLeft += speed;
		}
		if (keyboardState.at(DIK_Q) & state)
		{
			MoveUpDown -= speed * upDownIntensity;
		}
		if (keyboardState.at(DIK_E) & state)
		{
			MoveUpDown += speed * upDownIntensity;
		}
		if (keyboardState.at(DIK_R) & state)
		{
			ResetCamera();
		}
	}

	void SceneCamera::ReleaseInputs()
	{
		if (DxKeyboard)
		{
			DxKeyboard->Unacquire();
			DxKeyboard->Release();
			DxKeyboard = nullptr;
		}
	
		if (DxMouse)
		{
			DxMouse->Unacquire();
			DxMouse->Release();
			DxMouse = nullptr;
		}
	
		if (DxInput)
		{
			DxInput->Release();
			DxInput = nullptr;
		}
	
		DxLastMouseState = {};
	}

} // namespace lde

#include "DxCamera.h"
#include <algorithm>
#include <SDL_keyboard.h>

DX::Camera::Camera(int width, int height)
{
	m_Position = DirectX::XMVectorSet(0.0f, 8.0f, -15.0f, 1.0f);

	constexpr auto pitch_radians = DirectX::XMConvertToRadians(30.0f);
	Rotate(pitch_radians, 0.0f);

	m_AspectRatio = static_cast<float>(width) / height;
	CalculateProjection();
}

void DX::Camera::Update(float delta_time)
{
	auto inputs = SDL_GetKeyboardState(nullptr);
	float speed = delta_time * 20.0f;

	auto direction = DirectX::XMVectorZero();
	if (inputs[SDL_SCANCODE_W])
	{
		direction = DirectX::XMVectorAdd(direction, DirectX::XMVectorScale(this->Forward(), speed));
	}
	else if (inputs[SDL_SCANCODE_S])
	{
		direction = DirectX::XMVectorAdd(direction, DirectX::XMVectorScale(this->Forward(), -speed));
	}

	if (inputs[SDL_SCANCODE_A])
	{
		auto left = DirectX::XMVector3Cross(this->Forward(), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
		left = DirectX::XMVector3Normalize(left);
		direction = DirectX::XMVectorAdd(direction, DirectX::XMVectorScale(left, speed));
	}
	else if (inputs[SDL_SCANCODE_D])
	{
		auto right = DirectX::XMVector3Cross(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), this->Forward());
		right = DirectX::XMVector3Normalize(right);
		direction = DirectX::XMVectorAdd(direction, DirectX::XMVectorScale(right, speed));
	}

	if (inputs[SDL_SCANCODE_E])
	{
		auto worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		// Calculate the camera's local right vector using the cross product of the world up and camera forward vectors
		auto cameraRightVector = DirectX::XMVector3Cross(worldUp, this->Forward());
		cameraRightVector = DirectX::XMVector3Normalize(cameraRightVector);

		// Calculate the camera's local up vector using the cross product of the camera forward and right vectors
		auto cameraUpVector = DirectX::XMVector3Cross(this->Forward(), cameraRightVector);
		cameraUpVector = DirectX::XMVector3Normalize(cameraUpVector);

		direction = DirectX::XMVectorAdd(direction, DirectX::XMVectorScale(cameraUpVector, speed));
	}
	else if (inputs[SDL_SCANCODE_Q])
	{
		// Assuming you have the DirectX::XMVECTOR cameraForwardVector and a predefined world up vector (0, 1, 0)
		auto worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		// Calculate the camera's local right vector using the cross product of the world up and camera forward vectors
		auto cameraRightVector = DirectX::XMVector3Cross(worldUp, this->Forward());
		cameraRightVector = DirectX::XMVector3Normalize(cameraRightVector);

		// Calculate the camera's local up vector using the cross product of the camera forward and right vectors
		auto cameraUpVector = DirectX::XMVector3Cross(this->Forward(), cameraRightVector);
		cameraUpVector = DirectX::XMVector3Normalize(cameraUpVector);

		// Calculate the camera's local down vector by negating the local up vector
		auto cameraDownVector = DirectX::XMVectorNegate(cameraUpVector);

		direction = DirectX::XMVectorAdd(direction, DirectX::XMVectorScale(cameraDownVector, speed));
	}

	m_Position = DirectX::XMVectorAdd(direction, m_Position);
	CalculateView();
}

void DX::Camera::Rotate(float pitch_radians, float yaw_radians)
{
	m_PitchRadians += pitch_radians;
	m_YawRadians += yaw_radians;
	m_PitchRadians = std::clamp<float>(m_PitchRadians, -(DirectX::XM_PIDIV2 - 0.1f), DirectX::XM_PIDIV2 - 0.1f);

	CalculateView();
}

void DX::Camera::UpdateAspectRatio(int width, int height)
{
	// Calculate window aspect ratio
	m_AspectRatio = static_cast<float>(width) / height;
	CalculateProjection();
}

void DX::Camera::UpdateFov(float fov)
{
	m_FieldOfViewDegrees += fov;
	m_FieldOfViewDegrees = std::clamp(m_FieldOfViewDegrees, 0.1f, 179.9f);
	CalculateProjection();
}

void DX::Camera::CalculateProjection()
{
	// Convert degrees to radians
	auto field_of_view_radians = DirectX::XMConvertToRadians(m_FieldOfViewDegrees);

	// Calculate camera's perspective
	m_Projection = DirectX::XMMatrixPerspectiveFovLH(field_of_view_radians, m_AspectRatio, 0.1f, 100.0f);
}

void DX::Camera::CalculateView()
{
	// Calculate view
	DirectX::XMVECTOR look_at = DirectX::XMVectorAdd(m_Position, Forward());
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = DirectX::XMMatrixLookAtLH(m_Position, look_at, up);
}

DirectX::XMVECTOR DX::Camera::Forward()
{
	DirectX::XMVECTOR global_forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(m_PitchRadians, m_YawRadians, 0.0f);
	DirectX::XMVECTOR look_direction = XMVector3TransformCoord(global_forward, rotation_matrix);

	return DirectX::XMVector3Normalize(look_direction);
}
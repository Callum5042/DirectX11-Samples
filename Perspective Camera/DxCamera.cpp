#include "DxCamera.h"
#include <algorithm>

DX::Camera::Camera(int width, int height)
{
	Resize(width, height);

	constexpr auto pitch_radians = DirectX::XMConvertToRadians(30.0f);
	Rotate(pitch_radians, 0.0f);
}

void DX::Camera::Resize(int width, int height)
{
	auto fov = 50.0f;

	// Convert degrees to radians
	auto field_of_view_radians = DirectX::XMConvertToRadians(fov);

	// Calculate window aspect ratio
	auto window_aspect_ratio = static_cast<float>(width) / height;

	// Calculate camera's perspective
	m_Projection = DirectX::XMMatrixPerspectiveFovLH(field_of_view_radians, window_aspect_ratio, 0.01f, 100.0f);
}

void DX::Camera::Rotate(float pitch_radians, float yaw_radians)
{
	m_PitchRadians += pitch_radians;
	m_YawRadians += yaw_radians;
	m_PitchRadians = std::clamp<float>(m_PitchRadians, -(DirectX::XM_PIDIV2 - 0.1f), DirectX::XM_PIDIV2 - 0.1f);

	// Convert Spherical to Cartesian coordinates.
	const auto radius = -8.0f;
	auto rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(m_PitchRadians, m_YawRadians, 0);
	auto position = DirectX::XMVectorSet(0.0f, 0.0f, radius, 0.0f);
	position = XMVector3TransformCoord(position, rotation_matrix);

	// Calculate camera's view
	auto eye = position;
	auto at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = DirectX::XMMatrixLookAtLH(eye, at, up);
}

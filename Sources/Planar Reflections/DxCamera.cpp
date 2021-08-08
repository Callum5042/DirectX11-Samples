#include "DxCamera.h"
#include <algorithm>

DX::Camera::Camera(int width, int height)
{
	constexpr auto pitch_radians = DirectX::XMConvertToRadians(30.0f);
	Rotate(pitch_radians, 0.0f);

	m_AspectRatio = static_cast<float>(width) / height;
	CalculateProjection();
}

void DX::Camera::Rotate(float pitch_radians, float yaw_radians)
{
	m_PitchRadians += pitch_radians;
	m_YawRadians += yaw_radians;
	m_PitchRadians = std::clamp<float>(m_PitchRadians, -(DirectX::XM_PIDIV2 - 0.1f), DirectX::XM_PIDIV2 - 0.1f);

	// Convert Spherical to Cartesian coordinates.
	const auto radius = -16.0f;
	auto rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(m_PitchRadians, m_YawRadians, 0);
	auto position = DirectX::XMVectorSet(0.0f, 0.0f, radius, 0.0f);
	position = XMVector3TransformCoord(position, rotation_matrix);

	// Calculate camera's view
	auto eye = position;
	auto at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = DirectX::XMMatrixLookAtLH(eye, at, up);

	// Calculate camera position
	DirectX::XMStoreFloat3(&m_Position, position);
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
	m_Projection = DirectX::XMMatrixPerspectiveFovLH(field_of_view_radians, m_AspectRatio, 0.01f, 100.0f);
}

#pragma once

#include <DirectXMath.h>

namespace DX
{
	// Perspective orbital camera
	class Camera
	{
	public:
		Camera(int width, int height);
		virtual ~Camera() = default;

		// Move camera
		void Update(float delta_time);

		// Recalculates the view based on the pitch and yaw
		void Rotate(float pitch, float yaw);

		// Update aspect ratio
		void UpdateAspectRatio(int width, int height);

		// Set field of view
		void UpdateFov(float fov);

		// Get projection matrix
		constexpr DirectX::XMMATRIX GetProjection() { return m_Projection; }

		// Get view matrix
		constexpr DirectX::XMMATRIX GetView() { return m_View; }

		// Get position vector
		constexpr DirectX::XMVECTOR GetPosition() { return m_Position; }

		// Forward
		DirectX::XMVECTOR Forward();

	private:
		// Projection matrix
		DirectX::XMMATRIX m_Projection;

		// View matrix
		DirectX::XMMATRIX m_View;

		// Position matrix
		DirectX::XMVECTOR m_Position;

		// Camera pitch in radians
		float m_PitchRadians = 0.0f;

		// Camera yaw in radians
		float m_YawRadians = 0.0f;

		// Camera field of view in degrees
		float m_FieldOfViewDegrees = 50.0f;

		// Float aspect ratio
		float m_AspectRatio = 0.0f;

		// Recalculates the projection based on the new window size
		void CalculateProjection();

		// Recalculate camera view
		void CalculateView();
	};
}
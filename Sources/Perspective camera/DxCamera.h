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

		// Recalculates the projection based on the new window size
		void Resize(int width, int height);

		// Recalculates the view based on the pitch and yaw
		void Rotate(float pitch, float yaw);

		// Get projection matrix
		constexpr DirectX::XMMATRIX GetProjection() { return m_Projection; }

		// Get view matrix
		constexpr DirectX::XMMATRIX GetView() { return m_View; }

	private:
		// Projection matrix
		DirectX::XMMATRIX m_Projection;

		// View matrix
		DirectX::XMMATRIX m_View;

		// Camera pitch in radians
		float m_PitchRadians = 0.0f;

		// Camera yaw in radians
		float m_YawRadians = 0.0f;
	};
}
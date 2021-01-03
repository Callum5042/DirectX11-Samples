#pragma once

#include "DxRenderer.h"
#include <vector>

namespace DX
{
	struct Vertex
	{
		float x = 0;
		float y = 0;
		float z = 0;
	};

	class Model
	{
	public:
		Model(DX::Renderer* renderer);
		virtual ~Model() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		// Number of vertices to draw
		UINT m_VertexCount = 0;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
	};
}
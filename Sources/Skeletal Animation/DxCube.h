#pragma once

#include "DxRenderer.h"
#include <vector>
#include <DirectXColors.h>

namespace DX
{
	struct Colour1
	{
		float r = 0;
		float g = 0;
		float b = 0;
		float a = 0;
	};

	struct Vertex1
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Vertex colour
		Colour1 colour = {};
	};

	class Cube
	{
	public:
		Cube(DX::Renderer* renderer);
		virtual ~Cube() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		// Number of indices to draw
		UINT m_IndexCount = 0;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();
	};
}
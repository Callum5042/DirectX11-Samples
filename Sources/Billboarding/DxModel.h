#pragma once

#include "DxRenderer.h"
#include <vector>
#include <DirectXColors.h>

namespace DX
{
	struct Vertex
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Size
		float width = 0;
		float height = 0;
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

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Vertices
		std::vector<Vertex> m_Vertices;
	};
}
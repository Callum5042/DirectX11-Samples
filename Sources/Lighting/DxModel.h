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

		// Vertex normals
		float nx;
		float ny;
		float nz;
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

		// Vertices
		std::vector<Vertex> Vertices;

		// Indices
		std::vector<UINT> Indices;

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
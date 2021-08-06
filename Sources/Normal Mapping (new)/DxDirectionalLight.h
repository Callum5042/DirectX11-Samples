#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "Vertex.h"

namespace DX
{
	class DirectionalLight
	{
	public:
		DirectionalLight(DX::Renderer* renderer);
		virtual ~DirectionalLight() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		// Texture matrix
		DirectX::XMMATRIX Texture = DirectX::XMMatrixIdentity();

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Mesh data
		MeshData m_MeshData;
	};
}
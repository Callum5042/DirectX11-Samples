#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "Vertex.h"

namespace DX
{
	class Floor
	{
	public:
		Floor(DX::Renderer* renderer);
		virtual ~Floor() = default;

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

		// Texture resource
		ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture = nullptr;
		ComPtr<ID3D11ShaderResourceView> m_NormalTexture = nullptr;
		void LoadTexture();
	};
}
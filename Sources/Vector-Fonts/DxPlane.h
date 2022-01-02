#pragma once

#include "DxRenderer.h"
#include <vector>
#include <DirectXColors.h>
#include "Vertex.h"

namespace DX
{
	class Plane
	{
	public:
		Plane(DX::Renderer* renderer);
		virtual ~Plane() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		// Set texture
		void SetTexture(ID3D11ShaderResourceView* texture);

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

		// Texture resource
		ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture = nullptr;
	};
}
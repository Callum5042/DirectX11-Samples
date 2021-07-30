#pragma once

#include "DxRenderer.h"
#include "DxCamera.h"
#include "MeshData.h"
#include <vector>
#include <DirectXColors.h>

namespace DX
{
	class Sky
	{
	public:
		Sky(DX::Renderer* renderer);
		virtual ~Sky() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		// Number of indices to draw
		MeshData m_MeshData;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Texture resource
		ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture = nullptr;
		void LoadTexture();

		// Depth stencil
		ComPtr<ID3D11DepthStencilState> m_DepthStencilState = nullptr;
		void CreateDepthStencilState();

		// Raster state
		void CreateRasterState();
		ComPtr<ID3D11RasterizerState> m_RasterState = nullptr;
	};
}
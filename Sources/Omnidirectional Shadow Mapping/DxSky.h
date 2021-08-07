#pragma once

#include "DxRenderer.h"
#include "DxCamera.h"
#include <vector>
#include <DirectXColors.h>

namespace DX
{
	struct SkyVertex
	{
		float x = 0;
		float y = 0;
		float z = 0;
	};

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
		std::vector<SkyVertex> m_Vertices;
		std::vector<UINT> m_Indices;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Depth stencil
		ComPtr<ID3D11DepthStencilState> m_DepthStencilState = nullptr;
		void CreateDepthStencilState();

		// Raster state
		void CreateRasterState();
		ComPtr<ID3D11RasterizerState> m_RasterState = nullptr;

		// Generate sphere
		void GenerateSphere(float radius, UINT sliceCount, UINT stackCount);


		// Texture sampler
		ComPtr<ID3D11SamplerState> m_AnisotropicSampler = nullptr;
		void CreateAnisotropicFiltering();
	};
}
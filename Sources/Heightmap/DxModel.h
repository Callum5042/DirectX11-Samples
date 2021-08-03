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

		// Texture
		float u = 0;
		float v = 0;
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

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Heightmap texture
		ComPtr<ID3D11ShaderResourceView> m_HeightmapTexture = nullptr;
		void LoadHeightmap(ID3D11Device* d3dDevice);

		// Average heightmap data
		float Average(int i, int j, std::vector<float>& heightmap_data, int heightmap_size);
	};
}
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

		// Texture UV
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

		// Texture resource
		ComPtr<ID3D11ShaderResourceView> m_DiffuseTexture = nullptr;
		void LoadTexture();

		// Heightmap
		std::vector<float> m_Heightmap;
		std::vector<DirectX::XMFLOAT2> mPatchBoundsY;
		void CalcPatchBoundsY(UINT i, UINT j);

		int CellsPerPatch = 64;
		float m_HeightScale = 50.0f;
		float m_HeightmapWidth = 2049;
		float m_HeightmapHeight = 2049;
		float m_CellSpacing = 0.5f;

		float mNumPatchVertRows = 0;
		float mNumPatchVertCols = 0;
		float mNumPatchVertices = 0;
		float mNumPatchQuadFaces = 0;
	};
}
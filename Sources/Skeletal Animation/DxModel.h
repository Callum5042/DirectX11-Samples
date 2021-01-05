#pragma once

#include "DxRenderer.h"
#include <vector>
#include <DirectXColors.h>

#undef min
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace DX
{
	struct Colour
	{
		float r = 0;
		float g = 0;
		float b = 0;
		float a = 0;
	};

	struct Vertex
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Vertex colour
		Colour colour = {};

		// Weights
		float weight[4];

		// Bone index
		int bone[4];
	};

	struct BoneInfo
	{
		DirectX::XMMATRIX offset;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		std::vector<BoneInfo> bones;
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

		// Number of indices to draw
		UINT m_IndexCount = 0;
		Mesh m_Mesh;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Load FBX model
		void LoadFBX(std::string&& path);
	};
}
#pragma once

#include "DxRenderer.h"
#include "DxShader.h"
#include "DxCamera.h"
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

		// Weight
		float weight_x = 0;
		float weight_y = 0;
		float weight_z = 0;
		float weight_w = 0;

		// Joint
		int joint_x = 0;
		int joint_y = 0;
		int joint_z = 0;
		int joint_w = 0;
	};

	struct BoneData
	{
		std::string name;
		DirectX::XMMATRIX matrix;
	};

	struct ModelObjectData
	{
		int index_count;
		int index_start;
		int base_vertex;
		DirectX::XMMATRIX transformation;
		std::vector<BoneData> bones;
	};

	class Model
	{
	public:
		Model(DX::Renderer* renderer, DX::Shader* shader);
		virtual ~Model() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

	private:
		DX::Renderer* m_DxRenderer = nullptr;
		DX::Shader* m_DxShader = nullptr;

		// Model object data
		std::vector<ModelObjectData> m_ModelObjectData;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer(std::vector<Vertex> vertices);

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer(std::vector<UINT> indices);
	};
}
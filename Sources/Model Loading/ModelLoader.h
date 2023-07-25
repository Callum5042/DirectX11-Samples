#pragma once

#include "DxModel.h"
#include <string>
#include <vector>
#include <map>
#include <assimp\scene.h>

namespace Assimp
{
	// Vertex data
	struct Vertex
	{
		// Position
		float x = 0;
		float y = 0;
		float z = 0;

		// Weights
		float weight[4] = { 0, 0, 0, 0 };

		// Bone index
		int bone[4] = { 0, 0, 0, 0 };
	};

	// Bone data
	struct Bone
	{
		int id = 0;
		int parent_id = 0;
		std::string name;
		std::string parent_name;
		DirectX::XMMATRIX bind_pose;
		DirectX::XMMATRIX inverse_bind_pose;
	};

	// Mesh data
	struct Subset
	{
		std::string name;
		UINT total_index = 0;
		UINT start_index = 0;
		UINT base_vertex = 0;
		DirectX::XMMATRIX transformation;
	};

	// Model data
	struct Model
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		std::vector<Subset> subset;
		std::vector<Bone> bones;
	};

	// Model loader
	class Loader
	{
	public:
		Loader() = default;
		virtual ~Loader() = default;

		// Load model
		Assimp::Model Load(const std::string& path);

	private:
		Model m_ModelData;

		// Load mesh data
		void LoadMesh(const aiScene* scene);

		// Load mesh vertices
		void LoadMeshVertices(const aiMesh* mesh);

		// Load mesh indices
		UINT LoadMeshIndices(const aiMesh* mesh);
	};
}
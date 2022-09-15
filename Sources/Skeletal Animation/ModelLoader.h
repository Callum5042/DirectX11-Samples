#pragma once

#include "DxModel.h"
#include <assimp/scene.h>
#include <string>
#include <vector>
#include <map>

namespace ModelLoader
{
	bool Load(const std::string& path, DX::Mesh* mesh);
};

namespace Assimp
{
	// Vertex data
	struct Vertex
	{
		// Position
		float x = 0;
		float y = 0;
		float z = 0;

		// Bone weight
		//float weight_x = 0;
		//float weight_y = 0;
		//float weight_z = 0;
		//float weight_w = 0;

		//// Bone id
		//float bone_x = 0;
		//float bone_y = 0;
		//float bone_z = 0;
		//float bone_w = 0;

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
		DirectX::XMMATRIX inversebindmatrix;
	};

	// Mesh data
	struct Subset
	{
		std::string name;
		UINT total_index = 0;
		UINT start_index = 0;
		UINT base_vertex = 0;
	};

	// Model data
	struct Model
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		std::vector<Subset> subset;
		std::vector<Bone> bones;
		std::map<std::string, DX::AnimationClip> animations;
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

		std::map<std::string, Assimp::Bone> bonemap;

		// Load mesh data
		void LoadMesh(const aiScene* scene);

		// Load mesh vertices
		void LoadMeshVertices(const aiMesh* mesh);

		// Load mesh indices
		UINT LoadMeshIndices(const aiMesh* mesh);

		// Load bones
		void LoadMeshBones(const aiMesh* mesh);

		// Load animations
		void LoadAnimations(const aiScene* scene);
	};
}
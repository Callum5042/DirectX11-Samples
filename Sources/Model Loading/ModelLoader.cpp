#include "ModelLoader.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace
{
	DirectX::XMMATRIX ConvertToDirectXMatrix(aiMatrix4x4 matrix)
	{
		// Direct3D is row major
		DirectX::XMMATRIX _matrix(
			matrix.a1, matrix.b1, matrix.c1, matrix.d1,
			matrix.a2, matrix.b2, matrix.c2, matrix.d2,
			matrix.a3, matrix.b3, matrix.c3, matrix.d3,
			matrix.a4, matrix.b4, matrix.c4, matrix.d4);

		return _matrix;
	}
}

Assimp::Model Assimp::Loader::Load(const std::string& path)
{
	Assimp::Importer importer;

	// Assimp will remove bones that aren't connect to a vertex. We want all the bones loaded regardless as the bones can have children bones that are animated
	importer.SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);

	// Load model
	const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Fast | aiProcess_PopulateArmatureData);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		throw std::runtime_error("Could not load model");
	}

	LoadMesh(scene);

	return m_ModelData;
}

void Assimp::Loader::LoadMesh(const aiScene* scene)
{
	m_ModelData.subset.resize(scene->mNumMeshes);
	for (UINT i = 0; i < scene->mNumMeshes; ++i)
	{ 
		aiMesh* mesh = scene->mMeshes[i];

		// Set transformation
		m_ModelData.subset[i].transformation = ConvertToDirectXMatrix(scene->mRootNode->FindNode(mesh->mName)->mTransformation);

		// Set mesh name
		m_ModelData.subset[i].name = mesh->mName.C_Str();

		// Set start vertex and index
		m_ModelData.subset[i].base_vertex = static_cast<UINT>(m_ModelData.vertices.size());
		m_ModelData.subset[i].start_index = static_cast<UINT>(m_ModelData.indices.size());

		// Load mesh vertices
		LoadMeshVertices(mesh);

		// Load mesh indices
		UINT indices = LoadMeshIndices(mesh);

		// Set total indices for this mesh
		m_ModelData.subset[i].total_index = indices;
	}
}

void Assimp::Loader::LoadMeshVertices(const aiMesh* mesh)
{
	for (UINT i = 0; i < mesh->mNumVertices; ++i)
	{
		// Set the positions
		float x = static_cast<float>(mesh->mVertices[i].x);
		float y = static_cast<float>(mesh->mVertices[i].y);
		float z = static_cast<float>(mesh->mVertices[i].z);

		// Create a vertex to store the mesh's vertices temporarily
		Assimp::Vertex vertex = {};
		vertex.x = x;
		vertex.y = y;
		vertex.z = z;

		// Add the vertex to the vertices vector
		m_ModelData.vertices.push_back(vertex);
	}
}

UINT Assimp::Loader::LoadMeshIndices(const aiMesh* mesh)
{
	std::vector<UINT> indices;
	for (UINT i = 0; i < mesh->mNumFaces; ++i)
	{
		// Get the face
		const aiFace& face = mesh->mFaces[i];

		// Add the indices of the face to the vector
		for (UINT k = 0; k < face.mNumIndices; ++k)
		{
			indices.push_back(face.mIndices[k]);
		}
	}

	m_ModelData.indices.insert(m_ModelData.indices.end(), indices.begin(), indices.end());
	return static_cast<UINT>(indices.size());
}

#include "ModelLoader.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#undef min
#undef max

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
	const aiScene* scene = importer.ReadFile(path, aiProcess_MakeLeftHanded | aiProcess_PopulateArmatureData);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		throw std::runtime_error("Could not load model");
	}

	LoadMesh(scene);
	LoadAnimations(scene);

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

		// Set bone data
		LoadMeshBones(mesh);
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

		// If the mesh doesn't have any bone data then add weight of 1
		if (!mesh->HasBones())
		{
			vertex.weight[0] = 1.0f;
		}

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

void Assimp::Loader::LoadMeshBones(const aiMesh* mesh)
{
	// Set bone data
	std::vector<Bone> bones(mesh->mNumBones);
	for (UINT i = 0; i < mesh->mNumBones; ++i)
	{
		aiBone* bone = mesh->mBones[i];

		// Set bone data
		bones[i].id = i;
		bones[i].name = bone->mName.C_Str();
		bones[i].parent_name = bone->mNode->mParent->mName.C_Str();
		bones[i].bind_pose = ConvertToDirectXMatrix(bone->mNode->mTransformation);
		bones[i].inverse_bind_pose = ConvertToDirectXMatrix(bone->mOffsetMatrix);

		// Set to map
		bonemap[bones[i].name] = bones[i];

		// Set vertex bone id and weight influence
		for (UINT j = 0; j < bone->mNumWeights; ++j)
		{
			const aiVertexWeight& vertex_weight = bone->mWeights[j];
			UINT vertex_id = vertex_weight.mVertexId;
			ai_real weight = vertex_weight.mWeight;

			// A bone can influence multiple vertices
			Assimp::Vertex& vertex = m_ModelData.vertices[vertex_id];

			// We only want a vertex to be influences by a max of 4 bones
			const int MAX_BONE_INFLUENCE = 4;
			for (int k = 0; k < MAX_BONE_INFLUENCE; ++k)
			{
				if (vertex.weight[k] == 0.0)
				{
					vertex.weight[k] = weight;
					vertex.bone[k] = i;
					break;
				}
			}
		}
	}

	// Set bone hierarchy
	for (auto& bone : bones)
	{
		bone.parent_id = bonemap[bone.parent_name].id;
	}

	// Assign to bones
	m_ModelData.bones.insert(m_ModelData.bones.begin(), bones.begin(), bones.end());
}

void Assimp::Loader::LoadAnimations(const aiScene* scene)
{
	for (UINT i = 0; i < scene->mNumAnimations; ++i)
	{
		const aiAnimation* animation = scene->mAnimations[i];
		std::string name = animation->mName.C_Str();

		// Animation clip
		DX::AnimationClip clip;
		clip.ticks_per_second = static_cast<float>(animation->mTicksPerSecond);
		clip.BoneAnimations.resize(animation->mNumChannels);

		// Channel is the bones being animated
		for (UINT j = 0; j < animation->mNumChannels; ++j)
		{
			const aiNodeAnim* channel = animation->mChannels[j];
			std::string bone_name = channel->mNodeName.C_Str();
			int bone_id = bonemap[bone_name].id;

			// Frames of the bone
			for (UINT k = 0; k < channel->mNumPositionKeys; ++k)
			{
				float time = static_cast<float>(channel->mPositionKeys[k].mTime);
				const aiVector3D translation = channel->mPositionKeys[k].mValue;
				const aiQuaternion rotation = channel->mRotationKeys[k].mValue;
				const aiVector3D scale = channel->mScalingKeys[k].mValue;

				// Store the key frame
				DX::Keyframe frame;
				frame.TimePos = time;
				frame.Translation = DirectX::XMFLOAT3(translation.x, translation.y, translation.z);
				frame.RotationQuat = DirectX::XMFLOAT4(rotation.x, rotation.y, rotation.z, rotation.w);
				frame.Scale = DirectX::XMFLOAT3(scale.x, scale.y, scale.z);

				clip.BoneAnimations[bone_id].Keyframes.push_back(frame);
			}
		}

		// Save clip into map by name
		m_ModelData.animations[name] = clip;
	}
}

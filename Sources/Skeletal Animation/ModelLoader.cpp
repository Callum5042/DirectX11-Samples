#include "ModelLoader.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#undef min
#undef max

namespace
{
	DirectX::XMMATRIX ConvertToDirectXMatrix(aiMatrix4x4 matrix)
	{
		DirectX::XMMATRIX _matrix(
			matrix.a1, matrix.b1, matrix.c1, matrix.d1,
			matrix.a2, matrix.b2, matrix.c2, matrix.d2,
			matrix.a3, matrix.b3, matrix.c3, matrix.d3,
			matrix.a4, matrix.b4, matrix.c4, matrix.d4);

		return _matrix;
	}
}

bool ModelLoader::Load(const std::string& path, DX::Mesh* meshData)
{
	Assimp::Importer importer;
	auto scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded | aiProcess_PopulateArmatureData);

	// Load model
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
		throw std::runtime_error("Could not load model");
	}

	unsigned total_index = 0;
	unsigned base_vertex = 0;

	// Process data
	for (auto mesh_index = 0u; mesh_index < scene->mNumMeshes; ++mesh_index)
	{
		DX::Subset subset;
		subset.startIndex = total_index;
		subset.baseVertex = base_vertex;

		auto mesh = scene->mMeshes[mesh_index];

		std::string name = mesh->mName.C_Str();
		std::cout << name << '\n';

		// Load vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			base_vertex++;

			// Set the positions
			float x = static_cast<float>(mesh->mVertices[i].x);
			float y = static_cast<float>(mesh->mVertices[i].y);
			float z = static_cast<float>(mesh->mVertices[i].z);

			// Create a vertex to store the mesh's vertices temporarily
			DX::Vertex vertex;
			vertex.x = x;
			vertex.y = y;
			vertex.z = z;

			// Detect and write colours
			if (mesh->HasVertexColors(0))
			{
				auto assimp_colour = mesh->mColors[0][i];

				vertex.colour.r = assimp_colour.r;
				vertex.colour.g = assimp_colour.g;
				vertex.colour.b = assimp_colour.b;
				vertex.colour.a = assimp_colour.a;
			}

			// Add the vertex to the vertices vector
			meshData->vertices.push_back(vertex);
		}

		// Iterate over the faces of the mesh
		auto index_count = 0u;
		for (auto i = 0u; i < mesh->mNumFaces; ++i)
		{
			// Get the face
			const auto& face = mesh->mFaces[i];

			// Add the indices of the face to the vector
			for (auto k = 0u; k < face.mNumIndices; ++k)
			{
				total_index++;
				index_count++;
				meshData->indices.push_back(face.mIndices[k]);
			}
		}

		subset.totalIndex = index_count;
		meshData->subsets.push_back(subset);

		// Load bones
		for (auto bone_index = 0u; bone_index < mesh->mNumBones; ++bone_index)
		{
			auto ai_bone = mesh->mBones[bone_index];

			DX::BoneInfo boneInfo = {};
			boneInfo.name = ai_bone->mName.C_Str();
			boneInfo.parentName = ai_bone->mNode->mParent->mName.C_Str();
			meshData->bones.push_back(boneInfo);

			meshData->bones[bone_index].offset = ConvertToDirectXMatrix(ai_bone->mOffsetMatrix);
			meshData->bones[bone_index].transform = ConvertToDirectXMatrix(ai_bone->mNode->mTransformation);

			// Vertex weight data
			for (auto bone_weight_index = 0u; bone_weight_index < ai_bone->mNumWeights; bone_weight_index++)
			{
				auto vertexID = ai_bone->mWeights[bone_weight_index].mVertexId;
				auto weight = ai_bone->mWeights[bone_weight_index].mWeight;
				auto& vertex = meshData->vertices[vertexID];

				for (int vertex_weight_index = 0; vertex_weight_index < 4; ++vertex_weight_index)
				{
					if (vertex.weight[vertex_weight_index] == 0.0)
					{
						vertex.weight[vertex_weight_index] = weight;
						vertex.bone[vertex_weight_index] = bone_index;
						break;
					}
				}
			}
		}

		// Calculate parent
		for (int i = 0; i < meshData->bones.size(); ++i)
		{
			int parentId = 0;
			for (int j = 0; j < meshData->bones.size(); ++j)
			{
				if (meshData->bones[i].parentName == meshData->bones[j].name)
				{
					parentId = j;
					break;
				}
			}

			meshData->bones[i].parentId = parentId;
		}

		// Put bones into map
		for (auto& bone : meshData->bones)
		{
			meshData->bonemap[bone.name] = bone;
		}
	}

	// Load animations
	auto animation = scene->mAnimations[0];
	float duration = animation->mDuration;
	float ticksPerSecond = animation->mTicksPerSecond;

	for (auto animation_index = 0u; animation_index < scene->mNumAnimations; ++animation_index)
	{
		auto animation = scene->mAnimations[animation_index];
		auto ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);

		DX::AnimationClip clip;
		clip.BoneAnimations.resize(animation->mNumChannels);
		for (unsigned i = 0; i < animation->mNumChannels; ++i)
		{
			auto channel = animation->mChannels[i];
			std::string name = channel->mNodeName.C_Str();
			for (unsigned k = 0; k < channel->mNumPositionKeys; ++k)
			{
				auto time = channel->mPositionKeys[k].mTime;
				auto pos = channel->mPositionKeys[k].mValue;
				auto rotation = channel->mRotationKeys[k].mValue;
				auto scale = channel->mScalingKeys[k].mValue;

				DX::Keyframe frame;
				frame.TimePos = static_cast<float>(time);
				frame.Translation = DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
				frame.RotationQuat = DirectX::XMFLOAT4(rotation.x, rotation.y, rotation.z, rotation.w);
				frame.Scale = DirectX::XMFLOAT3(scale.x, scale.y, scale.z);

				clip.BoneAnimationsMap[name].Keyframes.push_back(frame);
			}
		}

		// Must put bones into array thing
		for (int i = 0; i < meshData->bones.size(); ++i)
		{
			std::string name = meshData->bones[i].name;
			clip.BoneAnimations[i] = clip.BoneAnimationsMap[name];
		}


		std::string animation_name = animation->mName.C_Str();
		meshData->animations["Take1"] = clip;
	}

	return true;
}

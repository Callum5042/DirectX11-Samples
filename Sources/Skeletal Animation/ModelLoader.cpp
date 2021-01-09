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
		aiVector3D scale;
		aiVector3D rot;
		aiVector3D pos;
		matrix.Decompose(scale, rot, pos);

		DirectX::XMMATRIX _matrix = DirectX::XMMatrixIdentity();
		// _matrix *= DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);

		_matrix *= DirectX::XMMatrixRotationX(rot.x);
		_matrix *= DirectX::XMMatrixRotationY(rot.y);
		_matrix *= DirectX::XMMatrixRotationZ(rot.z);


		_matrix *= DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		_matrix *= DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

		return _matrix;
	}
}

bool ModelLoader::Load(const std::string& path, DX::Mesh* dx_mesh)
{
	Assimp::Importer importer;
	auto scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded | aiProcess_PopulateArmatureData);

	// Load scene
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString();
		return false;
	}

	auto mesh = scene->mMeshes[0];

	// Process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
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
		dx_mesh->vertices.push_back(vertex);
	}

	// Iterate over the faces of the mesh
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		// Get the face
		aiFace face = mesh->mFaces[i];

		// Add the indices of the face to the vector
		for (unsigned int k = 0; k < face.mNumIndices; ++k)
		{
			dx_mesh->indices.push_back(face.mIndices[k]);
		}
	}

	// Load bones
	auto global = ConvertToDirectXMatrix(scene->mRootNode->mTransformation);
	DirectX::XMFLOAT4X4 _offset;
	DirectX::XMStoreFloat4x4(&_offset, global);

	for (auto bone_index = 0u; bone_index < mesh->mNumBones; ++bone_index)
	{
		auto ai_bone = mesh->mBones[bone_index];

		DX::BoneInfo boneInfo = {};
		boneInfo.name = ai_bone->mName.C_Str();
		boneInfo.parentName = ai_bone->mNode->mParent->mName.C_Str();
		dx_mesh->bones.push_back(boneInfo);

		auto offset = ConvertToDirectXMatrix(ai_bone->mOffsetMatrix);
		DirectX::XMFLOAT4X4 _offset;
		DirectX::XMStoreFloat4x4(&_offset, offset);
		dx_mesh->bones[bone_index].offset = _offset;

		// Vertex weight data
		for (auto bone_weight_index = 0u; bone_weight_index < ai_bone->mNumWeights; bone_weight_index++)
		{
			auto vertexID = ai_bone->mWeights[bone_weight_index].mVertexId;
			auto weight = ai_bone->mWeights[bone_weight_index].mWeight;
			auto& vertex = dx_mesh->vertices[vertexID];

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

	// Load animations
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

				clip.BoneAnimations[i].Keyframes.push_back(frame);
			}
		}

		//mAnimations["Take1"] = clip;
	}

	return true;
}

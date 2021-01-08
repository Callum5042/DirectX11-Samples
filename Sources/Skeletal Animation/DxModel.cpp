#include "DxModel.h"
#include <DirectXMath.h>
#include <vector>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <iostream>

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

DX::Model::Model(DX::Renderer* renderer, DX::Shader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
	auto world = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	DirectX::XMStoreFloat4x4(&World, world);
}

void DX::Model::Create()
{
	LoadFBX("D:\\bone.glb");
	//LoadFBX("..\\..\\Resources\\Models\\post_3bone.fbx");
	//LoadFBX("..\\..\\Resources\\Models\\side_3bone.glb");

	CreateVertexBuffer();
	CreateIndexBuffer();

	DirectX::XMVECTOR q0 = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), DirectX::XMConvertToRadians(0.0f));
	DirectX::XMVECTOR q1 = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), DirectX::XMConvertToRadians(180.0f));

	/*Animation.Keyframes.resize(3);
	Animation.Keyframes[0].TimePos = 0.0f;
	Animation.Keyframes[0].Translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	Animation.Keyframes[0].Scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	XMStoreFloat4(&Animation.Keyframes[0].RotationQuat, q0);

	Animation.Keyframes[1].TimePos = 2.0f;
	Animation.Keyframes[1].Translation = DirectX::XMFLOAT3(0.0f, -2.0f, 0.0f);
	Animation.Keyframes[1].Scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
	XMStoreFloat4(&Animation.Keyframes[1].RotationQuat, q1);

	Animation.Keyframes[2].TimePos = 8.0f;
	Animation.Keyframes[2].Translation = DirectX::XMFLOAT3(-7.0f, 0.0f, 0.0f);
	Animation.Keyframes[2].Scale = DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f);
	XMStoreFloat4(&Animation.Keyframes[2].RotationQuat, q0);*/
}

void DX::Model::Update(float dt)
{
	static float TimeInSeconds = 0.0f;
	TimeInSeconds += dt * 100.0f;
	std::cout << TimeInSeconds << '\n';

	auto numBones = m_Mesh.bones.size();
	std::vector<DirectX::XMFLOAT4X4> toParentTransforms(numBones);

	for (unsigned i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(TimeInSeconds, toParentTransforms[i]);
	}

	if (TimeInSeconds > BoneAnimations.back().Keyframes.back().TimePos)
	{
		TimeInSeconds = 0.0f;
	}

	std::vector<DirectX::XMFLOAT4X4> toRootTransforms(numBones);
	toRootTransforms[0] = toParentTransforms[0];

	for (UINT i = 1; i < numBones; ++i)
	{
		DirectX::XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = m_Mesh.bones[i].parentId;
		DirectX::XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		DirectX::XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pass bone data to pipeline
	////////////////////////////////////////////////////////////////////////////////////////////////////
	BoneBuffer bone_buffer = {};
	for (size_t i = 0; i < m_Mesh.bones.size(); i++)
	{
		DirectX::XMMATRIX offset = m_Mesh.bones[i].offset;
		DirectX::XMMATRIX toRoot = DirectX::XMLoadFloat4x4(&toRootTransforms[i]);

		DirectX::XMMATRIX matrix = DirectX::XMMatrixMultiply(offset, toRoot);
		//matrix = DirectX::XMMatrixIdentity();
		bone_buffer.transform[i] = matrix;
	}

	m_DxShader->UpdateBoneConstantBuffer(bone_buffer);
}

void DX::Model::CreateVertexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * m_Mesh.vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = m_Mesh.vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::CreateIndexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();
	m_IndexCount = static_cast<UINT>(m_Mesh.indices.size());

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * m_Mesh.indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = m_Mesh.indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_d3dIndexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::LoadFBX(std::string&& path)
{
	Scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded | aiProcess_PopulateArmatureData);

	// Load model
	if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
	{
		std::ostringstream ss;
		ss << "ERROR::ASSIMP::" << importer.GetErrorString();
		throw std::runtime_error("Could not load model");
	}

	// Process data
	auto mesh = Scene->mMeshes[0];

	// Load vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		// Set the positions
		float x = static_cast<float>(mesh->mVertices[i].x);
		float y = static_cast<float>(mesh->mVertices[i].y);
		float z = static_cast<float>(mesh->mVertices[i].z);

		// Create a vertex to store the mesh's vertices temporarily
		Vertex vertex;
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
		m_Mesh.vertices.push_back(vertex);
	}

	// Iterate over the faces of the mesh
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		// Get the face
		aiFace face = mesh->mFaces[i];

		// Add the indices of the face to the vector
		for (unsigned int k = 0; k < face.mNumIndices; ++k)
		{
			m_Mesh.indices.push_back(face.mIndices[k]);
		}
	}

	// Load bones
	for (auto bone_index = 0u; bone_index < mesh->mNumBones; ++bone_index)
	{
		auto ai_bone = mesh->mBones[bone_index];

		BoneInfo boneInfo = {};
		boneInfo.name = ai_bone->mName.C_Str();
		boneInfo.offset = ConvertToDirectXMatrix(ai_bone->mOffsetMatrix);
		m_Mesh.bones.push_back(boneInfo);

		// Vertex weight data
		for (auto bone_weight_index = 0u; bone_weight_index < ai_bone->mNumWeights; bone_weight_index++)
		{
			auto vertexID = ai_bone->mWeights[bone_weight_index].mVertexId;
			auto weight = ai_bone->mWeights[bone_weight_index].mWeight;
			auto& vertex = m_Mesh.vertices[vertexID];

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
	BoneAnimations.resize(m_Mesh.bones.size());
	for (auto animation_index = 0u; animation_index < Scene->mNumAnimations; ++animation_index)
	{
		auto animation = Scene->mAnimations[animation_index];
		//BoneAnimations.name = animation->mName.C_Str();
		auto ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);

		for (unsigned i = 0; i < animation->mNumChannels; ++i)
		{
			auto channel = animation->mChannels[i];
			std::string name = channel->mNodeName.C_Str();
			// Animation.boneAnimation[i].keyFrames.resize(channel->mNumPositionKeys);

			for (unsigned k = 0; k < channel->mNumPositionKeys; ++k)
			{
				auto time = channel->mPositionKeys[k].mTime;
				auto pos = channel->mPositionKeys[k].mValue;
				auto rotation = channel->mRotationKeys[k].mValue;
				auto scale = channel->mScalingKeys[k].mValue;

				Keyframe frame;
				frame.TimePos = static_cast<float>(time);
				frame.Translation = DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
				frame.RotationQuat = DirectX::XMFLOAT4(rotation.x, rotation.y, -rotation.z, -rotation.w);
				frame.Scale = DirectX::XMFLOAT3(scale.x, scale.y, scale.z);

				BoneAnimations[i].Keyframes.push_back(frame);
			}
		}
	}
}

void DX::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// We need the stride and offset for the vertex
	UINT vertex_stride = sizeof(Vertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// We need the stride and offset for the colour
	UINT colour_stride = sizeof(DirectX::XMVECTORF32);
	auto colour_offset = 0u;

	// Bind the index buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render geometry
	d3dDeviceContext->DrawIndexed(m_IndexCount, 0, 0);


	// Bone geometry
	//d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dBoneVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);
	//d3dDeviceContext->Draw(2, 0);
}

DX::Keyframe::Keyframe() : TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

DX::Keyframe::~Keyframe()
{
}

float DX::BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float DX::BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	float f = Keyframes.back().TimePos;

	return f;
}

void DX::BoneAnimation::Interpolate(float t, DirectX::XMFLOAT4X4& M)const
{
	if (t <= Keyframes.front().TimePos)
	{
		DirectX::XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		DirectX::XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		DirectX::XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		DirectX::XMVECTOR zero = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, DirectX::XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if (t >= Keyframes.back().TimePos)
	{
		DirectX::XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		DirectX::XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		DirectX::XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		DirectX::XMVECTOR zero = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, DirectX::XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for (UINT i = 0; i < Keyframes.size() - 1; ++i)
		{
			if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

				DirectX::XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				DirectX::XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].Scale);

				DirectX::XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				DirectX::XMVECTOR p1 = XMLoadFloat3(&Keyframes[i + 1].Translation);

				DirectX::XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				DirectX::XMVECTOR q1 = XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

				DirectX::XMVECTOR S = DirectX::XMVectorLerp(s0, s1, lerpPercent);
				DirectX::XMVECTOR P = DirectX::XMVectorLerp(p0, p1, lerpPercent);
				DirectX::XMVECTOR Q = DirectX::XMQuaternionSlerp(q0, q1, lerpPercent);

				DirectX::XMVECTOR zero = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, DirectX::XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}
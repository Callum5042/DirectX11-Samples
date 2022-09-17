#include "DxModel.h"
#include <DirectXMath.h>
#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <exception>
#include <fstream>

#include "ModelLoader.h"
#include "GltfModelLoader.h"
using namespace DX;

DX::Model::Model(DX::Renderer* renderer, DX::Shader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
	World = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
}

void DX::Model::Create()
{
	// Load model
	Assimp::Loader loader;
	Assimp::Model model = loader.Load("..\\..\\Resources\\Models\\man.gltf");

	// Assign vertices
	for (auto& v : model.vertices)
	{
		DX::Vertex vertex;

		// Set position
		vertex.x = v.x;
		vertex.y = v.y;
		vertex.z = v.z;

		// Set bone
		vertex.bone[0] = v.bone[0];
		vertex.bone[1] = v.bone[1];
		vertex.bone[2] = v.bone[2];
		vertex.bone[3] = v.bone[3];

		// Set weight
		vertex.weight[0] = v.weight[0];
		vertex.weight[1] = v.weight[1];
		vertex.weight[2] = v.weight[2];
		vertex.weight[3] = v.weight[3];

		m_Mesh.vertices.push_back(vertex);
	}

	// Assign indices
	m_Mesh.indices = model.indices;

	// Assign subset
	for (auto& s : model.subset)
	{
		DX::Subset subset = {};
		subset.baseVertex = s.base_vertex;
		subset.startIndex = s.start_index;
		subset.totalIndex = s.total_index;
		m_Mesh.subsets.push_back(subset);
	}

	// Assign bones
	for (auto& b : model.bones)
	{
		DX::BoneInfo bone;
		bone.name = b.name;
		bone.parentName = b.parent_name;
		bone.bind_pose = b.bind_pose;
		bone.inverse_bind_pose = b.inverse_bind_pose;
		bone.parentId = b.parent_id;
		m_Mesh.bones.push_back(bone);
	}

	m_Mesh.animations = model.animations;

	// Create buffers
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void DX::Model::Update(float dt)
{
	// https://stackoverflow.com/questions/62998968/how-do-i-calculate-the-start-matrix-for-each-bonet-pose-using-collada-and-ope

	// Time to interpolate each frame between
	static float time_in_seconds = 0.0f;

	// Parent transforms
	std::vector<DirectX::XMMATRIX> parent_transform(m_Mesh.bones.size());

	// Animation - Take the first one we find - we can also select the animation by name with "find"
	auto clip = m_Mesh.animations.begin();
	if (clip != m_Mesh.animations.end())
	{
		// Time to interpolate each frame between
		time_in_seconds += dt * clip->second.ticks_per_second;

		clip->second.Interpolate(time_in_seconds, parent_transform);
		if (time_in_seconds > clip->second.GetClipEndTime())
		{
			time_in_seconds = 0.0f;
		}
	}
	else
	{ 
		// If there is no animation then set the parent transform to the default bind pose
		for (size_t i = 0; i < m_Mesh.bones.size(); ++i)
		{
			parent_transform[i] = m_Mesh.bones[i].bind_pose;
		}
	}

	// Transform to root
	std::vector<DirectX::XMMATRIX> local_transform(m_Mesh.bones.size());
	local_transform[0] = parent_transform[0];
	for (UINT i = 1; i < m_Mesh.bones.size(); ++i)
	{
		DirectX::XMMATRIX parent = parent_transform[i];
		DirectX::XMMATRIX root = local_transform[m_Mesh.bones[i].parentId];
		local_transform[i] = XMMatrixMultiply(parent, root);
	}

	// Transform bone
	BoneBuffer bone_buffer = {}; 
	for (size_t i = 0; i < m_Mesh.bones.size(); i++)
	{
		DirectX::XMMATRIX inverse_bind_pose = m_Mesh.bones[i].inverse_bind_pose;
		DirectX::XMMATRIX local = local_transform[i];
		DirectX::XMMATRIX final_transform = DirectX::XMMatrixMultiply(inverse_bind_pose, local);

		bone_buffer.transform[i] = DirectX::XMMatrixTranspose(final_transform);
	}

	// Store final transform into bone buffer
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

void DX::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// We need the stride and offset for the vertex
	UINT vertex_stride = sizeof(Vertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// Bind the index buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render geometry
	for (auto i = 0u; i < m_Mesh.subsets.size(); ++i)
	{
		auto& subset = m_Mesh.subsets[i];
		d3dDeviceContext->DrawIndexed(subset.totalIndex, subset.startIndex, subset.baseVertex);
	}
}

float DX::BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float DX::BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	return Keyframes.back().TimePos;
}

void DX::BoneAnimation::Interpolate(float t, DirectX::XMMATRIX& M)const
{
	if (t <= Keyframes.front().TimePos)
	{
		// First frame so we got nothing to interpolate between
		DirectX::XMVECTOR scale = XMLoadFloat3(&Keyframes.front().Scale);
		DirectX::XMVECTOR translation = XMLoadFloat3(&Keyframes.front().Translation);
		DirectX::XMVECTOR rotation = XMLoadFloat4(&Keyframes.front().RotationQuat);

		DirectX::XMVECTOR origin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		M = DirectX::XMMatrixAffineTransformation(scale, origin, rotation, translation);
	}
	else if (t >= Keyframes.back().TimePos)
	{
		// Last frame so we got nothing to interpolate between
		DirectX::XMVECTOR scale = XMLoadFloat3(&Keyframes.back().Scale);
		DirectX::XMVECTOR translation = XMLoadFloat3(&Keyframes.back().Translation);
		DirectX::XMVECTOR rotation = XMLoadFloat4(&Keyframes.back().RotationQuat);

		DirectX::XMVECTOR origin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		M = DirectX::XMMatrixAffineTransformation(scale, origin, rotation, translation);
	}
	else
	{
		// Only want to interpolate between current frame and next frame determined by the frame time
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
				M = DirectX::XMMatrixAffineTransformation(S, zero, Q, P);

				// Exit early as we have found the frame to interpolate between
				break;
			}
		}
	}
}

void DX::BoneAnimation::Frame(int frame, DirectX::XMMATRIX& bone_transforms) const
{
	frame = std::clamp<int>(frame, 1, static_cast<int>(Keyframes.size())) - 1;

	DirectX::XMVECTOR scale = DirectX::XMLoadFloat3(&Keyframes[frame].Scale);
	DirectX::XMVECTOR translation = DirectX::XMLoadFloat3(&Keyframes[frame].Translation);
	DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&Keyframes[frame].RotationQuat);

	DirectX::XMVECTOR origin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	bone_transforms = DirectX::XMMatrixAffineTransformation(scale, origin, rotation, translation);
}

float DX::AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = FLT_MAX;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = std::min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float DX::AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = std::max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

void DX::AnimationClip::Interpolate(float t, std::vector<DirectX::XMMATRIX>& boneTransforms)const
{
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

void DX::AnimationClip::Frame(int frame, std::vector<DirectX::XMMATRIX>& bone_transforms) const
{
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Frame(frame, bone_transforms[i]);
	}
}

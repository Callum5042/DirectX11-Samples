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
	// Load data
	//ModelLoader::Load("..\\..\\Resources\\Models\\skinned_mesh.gltf", &m_Mesh);
	//ModelLoader::Load("..\\..\\Resources\\Models\\3bone.gltf", &m_Mesh);
	//ModelLoader::Load("..\\..\\Resources\\Models\\man.gltf", &m_Mesh);
	ModelLoader::Load("..\\..\\Resources\\Models\\test.gltf", &m_Mesh);

	GltfModelLoader loader;
	//auto fileData = loader.Load("..\\..\\Resources\\Models\\skinned_mesh.gltf");
	//auto fileData = loader.Load("..\\..\\Resources\\Models\\3bone.gltf");
	//auto fileData = loader.Load("..\\..\\Resources\\Models\\test.gltf");

	//m_Mesh.vertices = fileData.vertices;
	//m_Mesh.indices = fileData.indices;
	//m_Mesh.subsets = fileData.model_object_data;
	//m_Mesh.bones = fileData.bones;
	//m_Mesh.animations["Take1"] = fileData.animationClip;

	// Create buffers
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void DX::Model::Update(float dt)
{
	static float TimeInSeconds = 0.0f;
	TimeInSeconds += dt * 100.0f;

	auto numBones = m_Mesh.bones.size();
	std::vector<DirectX::XMMATRIX> toParentTransforms(numBones);

	// Animation
	auto clip = m_Mesh.animations.find("Take1");
	clip->second.Interpolate(TimeInSeconds, toParentTransforms);
	if (TimeInSeconds > clip->second.GetClipEndTime())
	{
		TimeInSeconds = 0.0f;
	}

	// Transform to root
	std::vector<DirectX::XMMATRIX> toRootTransforms(numBones);
	toRootTransforms[0] = toParentTransforms[0];
	for (UINT i = 1; i < numBones; ++i)
	{
		DirectX::XMMATRIX toParent = toParentTransforms[i];
		DirectX::XMMATRIX parentToRoot = toRootTransforms[m_Mesh.bones[i].parentId];
		toRootTransforms[i] = XMMatrixMultiply(toParent, parentToRoot);
	}

	// Transform bone

	finalTransform.clear();
	BoneBuffer bone_buffer = {};
	for (size_t i = 0; i < m_Mesh.bones.size(); i++)
	{
		DirectX::XMMATRIX offset = m_Mesh.bones[i].offset;
		DirectX::XMMATRIX toRoot = toRootTransforms[i];
		DirectX::XMMATRIX matrix = DirectX::XMMatrixMultiply(offset, toRoot);

		bone_buffer.transform[i] = DirectX::XMMatrixTranspose(matrix);
		finalTransform.push_back(toRoot);
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
		DirectX::XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		DirectX::XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		DirectX::XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		DirectX::XMVECTOR zero = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		M = DirectX::XMMatrixAffineTransformation(S, zero, Q, P);
	}
	else if (t >= Keyframes.back().TimePos)
	{
		DirectX::XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		DirectX::XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		DirectX::XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		DirectX::XMVECTOR zero = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		M = DirectX::XMMatrixAffineTransformation(S, zero, Q, P);
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
				M = DirectX::XMMatrixAffineTransformation(S, zero, Q, P);

				break;
			}
		}
	}
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
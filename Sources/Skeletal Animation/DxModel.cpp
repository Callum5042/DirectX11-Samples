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
		return *reinterpret_cast<DirectX::XMMATRIX*>(&matrix);
	}
}

DX::Model::Model(DX::Renderer* renderer, DX::Shader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
	World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	World *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(90.0f));
}

void DX::Model::Create()
{
	LoadFBX("..\\..\\Resources\\Models\\two_bone_animation.fbx");

	CreateVertexBuffer();
	CreateIndexBuffer();
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
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded | aiProcess_PopulateArmatureData);

	// Load model
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::ostringstream ss;
		ss << "ERROR::ASSIMP::" << importer.GetErrorString();
		throw std::runtime_error("Could not load model");
	}

	// Process data
	auto mesh = scene->mMeshes[0];

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

		// Add the vertex to the vertices vector
		m_Mesh.vertices.push_back(vertex);
	}

	// Colour vertices
	m_Mesh.vertices[0].colour.r = 1.0f;
	m_Mesh.vertices[1].colour.r = 1.0f;
	m_Mesh.vertices[2].colour.r = 1.0f;
	m_Mesh.vertices[3].colour.r = 1.0f;
	m_Mesh.vertices[33].colour.r = 1.0f;
	m_Mesh.vertices[32].colour.r = 1.0f;
	m_Mesh.vertices[29].colour.r = 1.0f;
	m_Mesh.vertices[28].colour.r = 1.0f;
	m_Mesh.vertices[25].colour.r = 1.0f;
	m_Mesh.vertices[24].colour.r = 1.0f;
	m_Mesh.vertices[26].colour.r = 1.0f;
	m_Mesh.vertices[27].colour.r = 1.0f;

	m_Mesh.vertices[40].colour.g = 1.0f;
	m_Mesh.vertices[41].colour.g = 1.0f;
	m_Mesh.vertices[35].colour.g = 1.0f;
	m_Mesh.vertices[34].colour.g = 1.0f;
	m_Mesh.vertices[36].colour.g = 1.0f;
	m_Mesh.vertices[37].colour.g = 1.0f;
	m_Mesh.vertices[44].colour.g = 1.0f;
	m_Mesh.vertices[45].colour.g = 1.0f;
	m_Mesh.vertices[49].colour.g = 1.0f;
	m_Mesh.vertices[48].colour.g = 1.0f;
	m_Mesh.vertices[14].colour.g = 1.0f;
	m_Mesh.vertices[15].colour.g = 1.0f;

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
	for (auto animation_index = 0u; animation_index < scene->mNumAnimations; ++animation_index)
	{
		auto animation = scene->mAnimations[animation_index];
		std::cout << "Animations frame rate: " << animation->mTicksPerSecond << '\n';


	}
}

void DX::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// Pass bone data to pipeline
	BoneBuffer bone_buffer = {};
	for (size_t i = 0; i < m_Mesh.bones.size(); i++)
	{
		auto offset = DirectX::XMMatrixIdentity();
		//bone_buffer.offset[i] = m_Mesh.bones[i].offset;
		bone_buffer.offset[i] = offset;
	}

	auto rotation = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(-45.0f));
	//bone_buffer.offset[1] = DirectX::XMMatrixMultiply(bone_buffer.offset[1], rotation);


	m_DxShader->UpdateBoneConstantBuffer(bone_buffer);

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
}

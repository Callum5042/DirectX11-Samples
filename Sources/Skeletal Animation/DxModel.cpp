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
		_matrix *= DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
		_matrix *= DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		_matrix *= DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

		return _matrix;
	}
}

DX::Model::Model(DX::Renderer* renderer, DX::Shader* shader) : m_DxRenderer(renderer), m_DxShader(shader)
{
	World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	//World *= DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(90.0f));
}

void DX::Model::Create()
{
	LoadFBX("..\\..\\Resources\\Models\\simple_sign.glb");
	//LoadFBX("..\\..\\Resources\\Models\\simple_sign.fbx");

	//World *= ConvertToDirectXMatrix(Scene->mRootNode->mTransformation);

	CreateVertexBuffer();
	CreateIndexBuffer();

	for (unsigned i = 0; i < Scene->mAnimations[0]->mNumChannels; ++i)
	{
		auto& bone = Scene->mAnimations[0]->mChannels[i];
		std::string name = bone->mNodeName.C_Str();

		if (name == "Bone.001")
		{
			for (size_t j = 0; j < bone->mNumRotationKeys; j++)
			{

				auto frame1 = bone->mRotationKeys[j].mValue;
				//auto frame2 = bone->mRotationKeys[9].mValue;

				std::cout << "Frame " << j << ": " << frame1.x << ", " << frame1.y << ", " << frame1.z << ", " << frame1.w << '\n';
				//std::cout << "Frame 2: " << frame2.x << ", " << frame2.y << ", " << frame2.z << ", " << frame2.w << '\n';
			}
		}
	}
}

const aiNodeAnim* DX::Model::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (unsigned i = 0; i < pAnimation->mNumChannels; i++) 
	{
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
		if (std::string(pNodeAnim->mNodeName.data) == NodeName) 
		{
			return pNodeAnim;
		}
	}

	return nullptr;
}

unsigned DX::Model::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


unsigned DX::Model::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


unsigned DX::Model::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

void DX::Model::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	unsigned NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void DX::Model::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	unsigned NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}


void DX::Model::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	unsigned NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void DX::Model::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const DirectX::XMMATRIX& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = Scene->mAnimations[0];
	DirectX::XMMATRIX NodeTransformation = ConvertToDirectXMatrix(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim) 
	{
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX ScalingM = DirectX::XMMatrixIdentity();
		//ScalingM *= DirectX::XMMatrixScaling(Scaling.x, Scaling.y, Scaling.z);
		ScalingM *= DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX RotationM = DirectX::XMMatrixIdentity();
		DirectX::XMVECTOR q = DirectX::XMVectorSet(RotationQ.x, RotationQ.y, RotationQ.z, RotationQ.w);
		RotationM *= DirectX::XMMatrixRotationQuaternion(q);

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX TranslationM = DirectX::XMMatrixIdentity();
		TranslationM *= DirectX::XMMatrixTranslation(Translation.x, Translation.y, Translation.z);

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	DirectX::XMMATRIX GlobalTransformation = ParentTransform * NodeTransformation;

	for (auto& bone : m_Mesh.bones)
	{
		if (bone.name == NodeName)
		{
			bone.transform = GlobalInverseTransform * GlobalTransformation * bone.offset;
		}
	}

	for (unsigned i = 0; i < pNode->mNumChildren; i++) 
	{
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

void DX::Model::Update(float dt)
{
	static float TimeInSeconds = 0.0f;
	TimeInSeconds += dt;

	auto numBones = m_Mesh.bones.size();

	DirectX::XMMATRIX Identity = DirectX::XMMatrixIdentity();
	float TicksPerSecond = (float)(Scene->mAnimations[0]->mTicksPerSecond != 0 ? Scene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, (float)Scene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, Scene->mRootNode, Identity);



	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pass bone data to pipeline
	////////////////////////////////////////////////////////////////////////////////////////////////////
	BoneBuffer bone_buffer = {};
	for (size_t i = 0; i < m_Mesh.bones.size(); i++)
	{
		bone_buffer.transform[i] = m_Mesh.bones[i].transform;
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

		// Add the vertex to the vertices vector
		m_Mesh.vertices.push_back(vertex);
	}

	// Colour vertices
	/*m_Mesh.vertices[0].colour.r = 1.0f;
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
	m_Mesh.vertices[15].colour.g = 1.0f;*/

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
	for (auto animation_index = 0u; animation_index < Scene->mNumAnimations; ++animation_index)
	{
		auto animation = Scene->mAnimations[animation_index];
		Animation.name = animation->mName.C_Str();
		Animation.ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);
		animation->mDuration;

		Animation.boneAnimation.resize(animation->mNumChannels);
		for (unsigned i = 0; i < animation->mNumChannels; ++i)
		{
			auto channel = animation->mChannels[i];
			Animation.boneAnimation[i].boneName = channel->mNodeName.C_Str();

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
				frame.RotationQuat = DirectX::XMFLOAT4(rotation.x, rotation.y, rotation.z, rotation.w);
				frame.Scale = DirectX::XMFLOAT3(scale.x, scale.y, scale.z);

				Animation.boneAnimation[i].keyFrames.push_back(frame);
			}
		}
	}

	auto globalTransform = ConvertToDirectXMatrix(Scene->mRootNode->mTransformation);
	GlobalInverseTransform = DirectX::XMMatrixInverse(nullptr, globalTransform);
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

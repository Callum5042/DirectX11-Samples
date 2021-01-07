#pragma once

#include "DxRenderer.h"
#include <vector>
#include <DirectXColors.h>
#include "DxShader.h"

#undef min
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace DX
{
	struct Colour
	{
		float r = 0;
		float g = 0;
		float b = 0;
		float a = 0;
	};

	struct Vertex
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Vertex colour
		Colour colour = {};

		// Weights
		float weight[4] = {0, 0, 0, 0};

		// Bone index
		int bone[4] = {0, 0, 0, 0};
	};

	struct BoneInfo
	{
		DirectX::XMMATRIX transform;
		DirectX::XMMATRIX offset;
		int parentId = 0;
		std::string name;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		std::vector<BoneInfo> bones;
	};

	struct Keyframe
	{
		Keyframe() {};
		~Keyframe() {};

		float TimePos = 0.0f;
		DirectX::XMFLOAT3 Translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		DirectX::XMFLOAT4 RotationQuat = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	};

	struct BoneAnimation
	{
		std::string boneName;
		std::vector<Keyframe> keyFrames;
	};

	struct AnimationData
	{
		std::string name;
		float ticksPerSecond = 0;

		std::vector<BoneAnimation> boneAnimation;
	};

	class Model
	{
	public:
		Model(DX::Renderer* renderer, DX::Shader* shader);
		virtual ~Model() = default;

		// Create device
		void Create();

		const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);

		unsigned FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);

		unsigned FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);

		unsigned FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);

		void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

		void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

		void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

		void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const DirectX::XMMATRIX& ParentTransform);

		// Update the model
		void Update(float dt);

		// Render the model
		void Render();

		// World 
		DirectX::XMMATRIX World = DirectX::XMMatrixIdentity();

		// Animation
		AnimationData Animation = {};

		// Scene
		Assimp::Importer importer;
		const aiScene* Scene = nullptr;

		DirectX::XMMATRIX GlobalInverseTransform = DirectX::XMMatrixIdentity();

	private:
		DX::Renderer* m_DxRenderer = nullptr;
		DX::Shader* m_DxShader = nullptr;

		// Number of indices to draw
		UINT m_IndexCount = 0;
		Mesh m_Mesh;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		ComPtr<ID3D11Buffer> m_d3dBoneVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Load FBX model
		void LoadFBX(std::string&& path);
	};
}
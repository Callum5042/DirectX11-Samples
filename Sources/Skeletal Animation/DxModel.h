#pragma once

#include "DxRenderer.h"
#include <vector>
#include <DirectXColors.h>
#include "DxShader.h"
#include <cmath>
#include <map>

#undef min
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#undef max

namespace DX
{
	struct Subset
	{
		Subset() :
			Id(-1),
			VertexStart(0), VertexCount(0),
			FaceStart(0), FaceCount(0)
		{
		}

		UINT Id;
		UINT VertexStart;
		UINT VertexCount;
		UINT FaceStart;
		UINT FaceCount;
	};

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
		float weight[4] = { 0, 0, 0, 0 };

		// Bone index
		int bone[4] = { 0, 0, 0, 0 };
	};

	struct BoneInfo
	{
		DirectX::XMMATRIX transform;
		DirectX::XMFLOAT4X4 offset;
		int parentId = 0;
		std::string name;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		std::vector<BoneInfo> bones;

		std::vector<DirectX::XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
	};

	///<summary>
	/// A Keyframe defines the bone transformation at an instant in time.
	///</summary>
	struct Keyframe
	{
		Keyframe();
		~Keyframe();

		float TimePos;
		DirectX::XMFLOAT3 Translation;
		DirectX::XMFLOAT3 Scale;
		DirectX::XMFLOAT4 RotationQuat;
	};

	///<summary>
	/// A BoneAnimation is defined by a list of keyframes.  For time
	/// values inbetween two keyframes, we interpolate between the
	/// two nearest keyframes that bound the time.  
	///
	/// We assume an animation always has two keyframes.
	///</summary>
	struct BoneAnimation
	{
		float GetStartTime() const;
		float GetEndTime() const;

		void Interpolate(float t, DirectX::XMFLOAT4X4& M) const;

		std::vector<Keyframe> Keyframes;
	};

	///<summary>
	/// Examples of AnimationClips are "Walk", "Run", "Attack", "Defend".
	/// An AnimationClip requires a BoneAnimation for every bone to form
	/// the animation clip.    
	///</summary>
	struct AnimationClip
	{
		float GetClipStartTime()const;
		float GetClipEndTime()const;

		void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const;

		std::vector<BoneAnimation> BoneAnimations;
	};

	class Model
	{
	public:
		Model(DX::Renderer* renderer, DX::Shader* shader);
		virtual ~Model() = default;

		// Create device
		void Create();

		// Update the model
		void Update(float dt);

		// Render the model
		void Render();

		// World 
		DirectX::XMFLOAT4X4 World;

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
		std::vector<DX::Subset> m_Subsets;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		ComPtr<ID3D11Buffer> m_d3dBoneVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Load FBX model
		void LoadFBX(std::string&& path);

		// Load m3d model
		void LoadM3d(const std::string& path);
		void ReadMaterials(std::ifstream& fin, UINT numMaterials);
		void ReadSubsetTable(std::ifstream& fin, UINT numSubsets);
		void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices);
		void ReadTriangles(std::ifstream& fin, UINT numTriangles);
		void ReadBoneOffsets(std::ifstream& fin, UINT numBones);
		void ReadBoneHierarchy(std::ifstream& fin, UINT numBones);
		void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips);
		void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);

		std::map<std::string, AnimationClip> mAnimations;
	};
}
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

	///<summary>
	/// A Keyframe defines the bone transformation at an instant in time.
	///</summary>
	struct Keyframe
	{
		Keyframe() = default;
		virtual ~Keyframe() = default;

		float TimePos = 0.0f;
		DirectX::XMFLOAT3 Translation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		DirectX::XMFLOAT3 Scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		DirectX::XMFLOAT4 RotationQuat = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
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

		void Interpolate(float t, DirectX::XMMATRIX& M) const;

		std::vector<Keyframe> Keyframes;
	};

	///<summary>
	/// Examples of AnimationClips are "Walk", "Run", "Attack", "Defend".
	/// An AnimationClip requires a BoneAnimation for every bone to form
	/// the animation clip.    
	///</summary>
	struct AnimationClip
	{
		float GetClipStartTime() const;
		float GetClipEndTime() const;

		void Interpolate(float t, std::vector<DirectX::XMMATRIX>& boneTransforms)const;

		std::vector<BoneAnimation> BoneAnimations;
	};

	struct BoneInfo
	{
		int parentId = 0;
		std::string name;
		std::string parentName;
		DirectX::XMMATRIX offset;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
		std::vector<BoneInfo> bones;
		std::map<std::string, AnimationClip> animations;
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
		DirectX::XMMATRIX World;

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
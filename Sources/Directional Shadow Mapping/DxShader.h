#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "DxModel.h"
#include <string>

namespace DX
{
	struct CameraBuffer
	{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
		DirectX::XMFLOAT3 cameraPosition;
		float padding;
	};

	struct WorldBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInverse;
	};

	struct DirectionalLightBuffer
	{
		DirectX::XMFLOAT4 direction;
	};

	class Shader
	{
	public:
		Shader(Renderer* renderer);
		virtual ~Shader() = default;

		// Create vertex shader
		void LoadVertexShader(std::string&& vertex_shader_path);

		// Create pixel shader
		void LoadPixelShader(std::string&& pixel_shader_path);

		// Bind the shader to the pipeline
		void Use();

		// Update camera buffer
		void UpdateCameraBuffer(const CameraBuffer& buffer);

		// Set world constant buffer from camera
		void UpdateWorldBuffer(const DirectX::XMMATRIX& world);

		// Update camera buffer
		void UpdateDirectionalLightBuffer(const DirectionalLightBuffer& buffer);

	private:
		Renderer* m_DxRenderer = nullptr;

		// Vertex shader
		ComPtr<ID3D11VertexShader> m_d3dVertexShader = nullptr;

		// Vertex shader input layout
		ComPtr<ID3D11InputLayout> m_d3dVertexLayout = nullptr;

		// Pixel shader
		ComPtr<ID3D11PixelShader> m_d3dPixelShader = nullptr;

		// Camera constant buffer
		ComPtr<ID3D11Buffer> m_d3dCameraConstantBuffer = nullptr;
		void CreateCameraConstantBuffer();

		// World constant buffer
		ComPtr<ID3D11Buffer> m_d3dWorldConstantBuffer = nullptr;
		void CreateWorldConstantBuffer();

		// Light constant buffer
		ComPtr<ID3D11Buffer> m_d3dDirectionalLightConstantBuffer = nullptr;
		void CreateDirectionalLightConstantBuffer();
	};
}
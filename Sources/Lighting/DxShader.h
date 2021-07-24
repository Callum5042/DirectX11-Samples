#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "DxModel.h"
#include <string>

namespace DX
{
	struct WorldBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
		DirectX::XMMATRIX worldInverse;
	};

	struct DirectionalLight
	{
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 specular;

		DirectX::XMFLOAT4 cameraPosition;
	};

	struct LightBuffer
	{
		DirectionalLight directionalLight;
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

		// Set world constant buffer from camera
		void UpdateWorldConstantBuffer(const WorldBuffer& worldBuffer);

		// Set light constant buffer
		void UpdateLightConstantBuffer(const LightBuffer& lightBuffer);

	private:
		Renderer* m_DxRenderer = nullptr;

		// Vertex shader
		ComPtr<ID3D11VertexShader> m_d3dVertexShader = nullptr;

		// Vertex shader input layout
		ComPtr<ID3D11InputLayout> m_d3dVertexLayout = nullptr;

		// Pixel shader
		ComPtr<ID3D11PixelShader> m_d3dPixelShader = nullptr;

		// World constant buffer
		ComPtr<ID3D11Buffer> m_d3dWorldConstantBuffer = nullptr;
		void CreateWorldConstantBuffer();

		// Light constant buffer
		ComPtr<ID3D11Buffer> m_d3dLightConstantBuffer = nullptr;
		void CreateLightConstantBuffer();
	};
}
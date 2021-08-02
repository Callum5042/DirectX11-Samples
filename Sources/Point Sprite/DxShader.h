#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include <string>

namespace DX
{
	struct WorldBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	class Shader
	{
	public:
		Shader(Renderer* renderer);
		virtual ~Shader() = default;

		// Create vertex shader
		void LoadVertexShader(std::string&& vertex_shader_path);

		// Create geometry shader
		void LoadGeometryShader(std::string&& geometry_shader_path);

		// Create pixel shader
		void LoadPixelShader(std::string&& pixel_shader_path);

		// Bind the shader to the pipeline
		void Use();

		// Set world constant buffer from camera
		void UpdateWorldConstantBuffer(const WorldBuffer& worldBuffer);

	private:
		Renderer* m_DxRenderer = nullptr;

		// Vertex shader
		ComPtr<ID3D11VertexShader> m_d3dVertexShader = nullptr;

		// Vertex shader input layout
		ComPtr<ID3D11InputLayout> m_d3dVertexLayout = nullptr;

		// Geometry shader
		ComPtr<ID3D11GeometryShader> m_d3dGeometryShader = nullptr;

		// Pixel shader
		ComPtr<ID3D11PixelShader> m_d3dPixelShader = nullptr;

		// World constant buffer
		ComPtr<ID3D11Buffer> m_d3dWorldConstantBuffer = nullptr;
		void CreateWorldConstantBuffer();
	};
}
#include "DxShader.h"
#include <fstream>
#include <vector>

DX::Shader::Shader(Renderer* renderer) : m_DxRenderer(renderer)
{
	CreateWorldConstantBuffer();
}

void DX::Shader::LoadVertexShader(std::string&& vertex_shader_path)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Load the binary file into memory
	std::ifstream file(vertex_shader_path, std::fstream::in | std::fstream::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Create the vertex shader
	DX::Check(d3dDevice->CreateVertexShader(data.data(), data.size(), nullptr, m_d3dVertexShader.ReleaseAndGetAddressOf()));

	// Describe the memory layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(layout);
	DX::Check(d3dDevice->CreateInputLayout(layout, numElements, data.data(), data.size(), m_d3dVertexLayout.ReleaseAndGetAddressOf()));
}

void DX::Shader::LoadGeometryShader(std::string&& geometry_shader_path)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Load the binary file into memory
	std::ifstream file(geometry_shader_path, std::fstream::in | std::fstream::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Create pixel shader
	DX::Check(d3dDevice->CreateGeometryShader(data.data(), data.size(), nullptr, m_d3dGeometryShader.ReleaseAndGetAddressOf()));
}

void DX::Shader::LoadPixelShader(std::string&& pixel_shader_path)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Load the binary file into memory
	std::ifstream file(pixel_shader_path, std::fstream::in | std::fstream::binary);
	std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Create pixel shader
	DX::Check(d3dDevice->CreatePixelShader(data.data(), data.size(), nullptr, m_d3dPixelShader.ReleaseAndGetAddressOf()));
}

void DX::Shader::Use()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// Bind the input layout to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetInputLayout(m_d3dVertexLayout.Get());

	// Bind the vertex shader to the pipeline's Vertex Shader stage
	d3dDeviceContext->VSSetShader(m_d3dVertexShader.Get(), nullptr, 0);

	// Bind the geometry shader to the pipeline's Geometry Shader stage
	d3dDeviceContext->GSSetShader(m_d3dGeometryShader.Get(), nullptr, 0);

	// Bind the pixel shader to the pipeline's Pixel Shader stage
	d3dDeviceContext->PSSetShader(m_d3dPixelShader.Get(), nullptr, 0);

	// Bind the world constant buffer to the geometry shader
	d3dDeviceContext->GSSetConstantBuffers(0, 1, m_d3dWorldConstantBuffer.GetAddressOf());
}

void DX::Shader::UpdateWorldConstantBuffer(const WorldBuffer& worldBuffer)
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();
	d3dDeviceContext->UpdateSubresource(m_d3dWorldConstantBuffer.Get(), 0, nullptr, &worldBuffer, 0, 0);
}

void DX::Shader::CreateWorldConstantBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create world constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WorldBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(d3dDevice->CreateBuffer(&bd, nullptr, m_d3dWorldConstantBuffer.ReleaseAndGetAddressOf()));
}

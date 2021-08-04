#include "DxShader.h"
#include <fstream>
#include <vector>

DX::Shader::Shader(Renderer* renderer) : m_DxRenderer(renderer)
{
	CreateCameraConstantBuffer();
	CreateWorldConstantBuffer();
	CreateDirectionalLightConstantBuffer();
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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);
	DX::Check(d3dDevice->CreateInputLayout(layout, numElements, data.data(), data.size(), m_d3dVertexLayout.ReleaseAndGetAddressOf()));
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

	// Bind the pixel shader to the pipeline's Pixel Shader stage
	d3dDeviceContext->PSSetShader(m_d3dPixelShader.Get(), nullptr, 0);

	// Bind the world constant buffer to the vertex shader
	d3dDeviceContext->VSSetConstantBuffers(0, 1, m_d3dCameraConstantBuffer.GetAddressOf());
	d3dDeviceContext->VSSetConstantBuffers(1, 1, m_d3dWorldConstantBuffer.GetAddressOf());

	// Bind the light constant buffer to pixel shader
	d3dDeviceContext->PSSetConstantBuffers(0, 1, m_d3dCameraConstantBuffer.GetAddressOf());
	d3dDeviceContext->PSSetConstantBuffers(2, 1, m_d3dDirectionalLightConstantBuffer.GetAddressOf());
}

void DX::Shader::UpdateCameraBuffer(const CameraBuffer& buffer)
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();
	d3dDeviceContext->UpdateSubresource(m_d3dCameraConstantBuffer.Get(), 0, nullptr, &buffer, 0, 0);
}

void DX::Shader::UpdateWorldBuffer(const DirectX::XMMATRIX& world)
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	DX::WorldBuffer buffer = {};
	buffer.world = DirectX::XMMatrixTranspose(world);
	buffer.worldInverse = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world));

	// We use Map/Unmap here over UpdateSubresource for performance
	D3D11_MAPPED_SUBRESOURCE resource = {};
	DX::Check(d3dDeviceContext->Map(m_d3dWorldConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
	std::memcpy(resource.pData, &buffer, sizeof(WorldBuffer));
	d3dDeviceContext->Unmap(m_d3dWorldConstantBuffer.Get(), 0);
}

void DX::Shader::UpdateDirectionalLightBuffer(const DirectionalLightBuffer& buffer)
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();
	d3dDeviceContext->UpdateSubresource(m_d3dDirectionalLightConstantBuffer.Get(), 0, nullptr, &buffer, 0, 0);
}

void DX::Shader::CreateCameraConstantBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create world constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CameraBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(d3dDevice->CreateBuffer(&bd, nullptr, m_d3dCameraConstantBuffer.ReleaseAndGetAddressOf()));
}

void DX::Shader::CreateWorldConstantBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create world constant buffer - Must set D3D11_USAGE_DYNAMIC and D3D11_CPU_ACCESS_WRITE to be able to with Map
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(WorldBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DX::Check(d3dDevice->CreateBuffer(&bd, nullptr, m_d3dWorldConstantBuffer.ReleaseAndGetAddressOf()));
}

void DX::Shader::CreateDirectionalLightConstantBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create light constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DirectionalLightBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX::Check(d3dDevice->CreateBuffer(&bd, nullptr, m_d3dDirectionalLightConstantBuffer.ReleaseAndGetAddressOf()));
}

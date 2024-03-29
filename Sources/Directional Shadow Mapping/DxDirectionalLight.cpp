#include "DxDirectionalLight.h"
#include <DirectXMath.h>
#include <SDL.h>
#include "GeometryGenerator.h"

DX::DirectionalLight::DirectionalLight(DX::Renderer* renderer) : m_DxRenderer(renderer)
{
	Position = DirectX::XMFLOAT3(5.0f, 4.0f, 6.0f);
}

void DX::DirectionalLight::Create()
{
	GeometryGenerator::CreateBox(0.2f, 0.2f, 0.2f, &m_MeshData);

	// Create input buffers
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void DX::DirectionalLight::CreateVertexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * m_MeshData.vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = m_MeshData.vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::DirectionalLight::CreateIndexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * m_MeshData.indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = m_MeshData.indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_d3dIndexBuffer.ReleaseAndGetAddressOf()));
}

void DX::DirectionalLight::Update(float delta_time)
{
	auto inputs = SDL_GetKeyboardState(nullptr);

	// Move forward/backward along Z-axis
	if (inputs[SDL_SCANCODE_W])
	{
		Position.z += 1.0f * delta_time;
	}
	else if (inputs[SDL_SCANCODE_S])
	{
		Position.z -= 1.0f * delta_time;
	}

	// Move left/right along X-axis
	if (inputs[SDL_SCANCODE_A])
	{
		Position.x += 1.0f * delta_time;
	}
	else if (inputs[SDL_SCANCODE_D])
	{
		Position.x -= 1.0f * delta_time;
	}

	// Move up/down along Y-axis v
	if (inputs[SDL_SCANCODE_E])
	{
		Position.y += 1.0f * delta_time;
	}
	else if (inputs[SDL_SCANCODE_Q])
	{
		Position.y -= 1.0f * delta_time;
	}

	World = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
}

void DX::DirectionalLight::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// We need the stride and offset for the vertex
	UINT vertex_stride = sizeof(Vertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the Input Assembler
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// Bind the index buffer to the Input Assembler
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the Input Assembler
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render geometry
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(m_MeshData.indices.size()), 0, 0);
}

DirectX::XMVECTOR DX::DirectionalLight::GetDirection()
{
	auto direction = DirectX::XMLoadFloat3(&Position);
	return DirectX::XMVector3Normalize(direction);
}

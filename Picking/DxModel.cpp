#include "DxModel.h"
#include <DirectXMath.h>
#include <vector>

DX::Model::Model(DX::Renderer* renderer) : m_DxRenderer(renderer)
{
	World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
}

void DX::Model::Create()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void DX::Model::CreateVertexBufferAgain()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * Vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = Vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::CreateVertexBuffer()
{
	// Set vertex data
	Vertices =
	{
		{ -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		{ +1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f }
	};

	CreateVertexBufferAgain();
}

void DX::Model::CreateIndexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Set Indices
	Indices =
	{
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7,
		8, 9, 10,
		8, 10, 11,
		12, 13, 14,
		12, 14, 15,
		16, 17, 18,
		16, 18, 19,
		20, 21, 22,
		20, 22, 23,
	};

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * Indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = Indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_d3dIndexBuffer.ReleaseAndGetAddressOf()));
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
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(Indices.size()), 0, 0);
}
 
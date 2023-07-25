#include "DxOverlay.h"
#include <DirectXMath.h>
#include "GeometryGenerator.h"

DX::Overlay::Overlay(DX::Renderer* renderer) : m_DxRenderer(renderer)
{
}

void DX::Overlay::Create()
{
	World = DirectX::XMMatrixTranslation(0.0f, 4.0f, 0.0f);

	// GeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f, &m_MeshData);

	m_Vertices =
	{
		{ -1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f  },
		{ +1.0f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f  },
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f  },
		{ +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f  },
	};

	// Double-sided
	m_Indices =
	{
		0, 1, 2,
		2, 1, 3,

		0, 2, 1,
		1, 2, 3
	};


	// Create input buffers
	CreateVertexBuffer();
	CreateIndexBuffer();
}

void DX::Overlay::CreateVertexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * m_Vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = m_Vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Overlay::CreateIndexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * m_Indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = m_Indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_d3dIndexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Overlay::Render()
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
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(m_Indices.size()), 0, 0);
}

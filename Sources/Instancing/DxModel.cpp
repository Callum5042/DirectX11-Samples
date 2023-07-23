#include "DxModel.h"
#include <DirectXMath.h>
#include <vector>

DX::Model::Model(DX::Renderer* renderer) : m_DxRenderer(renderer)
{

}

void DX::Model::Create()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateInstanceData();
}

void DX::Model::CreateVertexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Set vertex data
	std::vector<Vertex> vertices =
	{
		{ -1.0f, -1.0f, -1.0f },
		{ -1.0f, +1.0f, -1.0f },
		{ +1.0f, +1.0f, -1.0f },
		{ +1.0f, -1.0f, -1.0f },
		{ -1.0f, -1.0f, +1.0f },
		{ +1.0f, -1.0f, +1.0f },
		{ +1.0f, +1.0f, +1.0f },
		{ -1.0f, +1.0f, +1.0f },
		{ -1.0f, +1.0f, -1.0f },
		{ -1.0f, +1.0f, +1.0f },
		{ +1.0f, +1.0f, +1.0f },
		{ +1.0f, +1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ +1.0f, -1.0f, -1.0f },
		{ +1.0f, -1.0f, +1.0f },
		{ -1.0f, -1.0f, +1.0f },
		{ -1.0f, -1.0f, +1.0f },
		{ -1.0f, +1.0f, +1.0f },
		{ -1.0f, +1.0f, -1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ +1.0f, -1.0f, -1.0f },
		{ +1.0f, +1.0f, -1.0f },
		{ +1.0f, +1.0f, +1.0f },
		{ +1.0f, -1.0f, +1.0f }
	};

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::CreateIndexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Set Indices
	std::vector<UINT> indices =
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

	m_IndexCount = static_cast<UINT>(indices.size());

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = indices.data();

	DX::Check(d3dDevice->CreateBuffer(&index_buffer_desc, &index_subdata, m_d3dIndexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::CreateInstanceData()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	auto world_1 = DirectX::XMMatrixTranslation(-3.0f, 0.0f, 0.0f);
	auto world_2 = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	auto world_3 = DirectX::XMMatrixTranslation(3.0f, 0.0f, 0.0f);

	m_InstanceData =
	{
		{ { 1.0f, 0.0f, 0.0f, 1.0f }, DirectX::XMMatrixTranspose(world_1) },
		{ { 0.0f, 1.0f, 0.0f, 1.0f }, DirectX::XMMatrixTranspose(world_2) },
		{ { 0.0f, 0.0f, 1.0f, 1.0f }, DirectX::XMMatrixTranspose(world_3) },
	};

	// Create instance data buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(VertexInstanceData) * 10000);
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	/*D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = m_InstanceData.data();*/

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, nullptr, m_d3dInstanceDataBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// Bind the vertex buffer to the pipeline's Input Assembler stage
	ID3D11Buffer* buffers[2] = { m_d3dVertexBuffer.Get(), m_d3dInstanceDataBuffer.Get() };
	UINT vertex_stride[2] = { sizeof(Vertex), sizeof(VertexInstanceData) };
	UINT vertex_offset[2] = { 0, 0 };

	d3dDeviceContext->IASetVertexBuffers(0, 2, buffers, vertex_stride, vertex_offset);

	// Bind the index buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render geometry
	UINT render_count = m_InstanceData.size();
	d3dDeviceContext->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);
	// d3dDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void DX::Model::Add()
{
	m_XDistance += 3;

	auto world = DirectX::XMMatrixTranslation((float)m_XDistance, 0.0f, 0.0f);
	m_InstanceData.push_back({ { 0.0f, 0.0f, 1.0f, 1.0f }, DirectX::XMMatrixTranspose(world) });

	// Map new resource
	D3D11_MAPPED_SUBRESOURCE resource = {};
	int memorysize = sizeof(VertexInstanceData) * static_cast<int>(m_InstanceData.size());

	DX::Check(m_DxRenderer->GetDeviceContext()->Map(m_d3dInstanceDataBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));
	std::memcpy(resource.pData, m_InstanceData.data(), memorysize);
	m_DxRenderer->GetDeviceContext()->Unmap(m_d3dInstanceDataBuffer.Get(), 0);
}

#include "DxModel.h"
#include "GltfModelLoader.h"
#include <DirectXMath.h>
#include <vector>

DX::Model::Model(DX::Renderer* renderer, DX::Shader* shader, DX::Camera* camera) : m_DxRenderer(renderer), m_DxShader(shader), m_DxCamera(camera)
{
	World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
}

void DX::Model::Create()
{
	// Load model
	GltfModelLoader loader;
	//GltfFileData file_data = loader.Load("D:\\Sources\\DirectX11 Samples\\Resources\\Models\\pyramid.gltf");
	//GltfFileData file_data = loader.Load("D:\\Sources\\DirectX11 Samples\\Resources\\Models\\monkey.gltf");
	GltfFileData file_data = loader.Load("D:\\Sources\\DirectX11 Samples\\Resources\\Models\\test.gltf");

	// Copy model object data
	m_ModelObjectData = file_data.model_object_data;

	// Create buffers
	CreateVertexBuffer(file_data.vertices);
	CreateIndexBuffer(file_data.indices);
}

void DX::Model::CreateVertexBuffer(std::vector<Vertex> vertices)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Model::CreateIndexBuffer(std::vector<UINT> indices)
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC index_buffer_desc = {};
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA index_subdata = {};
	index_subdata.pSysMem = indices.data();

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

	// Bind the index buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render all geometry
	for (auto& obj : m_ModelObjectData)
	{
		// Set obj transformation
		auto matrix = DirectX::XMMatrixMultiply(World, obj.transformation);

		// Apply object transformation
		DX::WorldBuffer world_buffer = {};
		world_buffer.world = DirectX::XMMatrixTranspose(obj.transformation);
		world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
		world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());

		m_DxShader->UpdateWorldConstantBuffer(world_buffer);

		// Render object
		d3dDeviceContext->DrawIndexed(obj.index_count, obj.index_start, obj.base_vertex);
	}

	// Set obj transformation
	//{
	//	auto matrix = DirectX::XMMatrixMultiply(World, m_ModelObjectData[0].transformation);

	//	// Apply object transformation
	//	DX::WorldBuffer world_buffer = {};
	//	world_buffer.world = DirectX::XMMatrixTranspose(matrix);
	//	world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
	//	world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());

	//	m_DxShader->UpdateWorldConstantBuffer(world_buffer);

	//	// Render object
	//	d3dDeviceContext->DrawIndexed(3456, 0, 0);
	//}

	//{
	//	auto matrix = DirectX::XMMatrixMultiply(World, m_ModelObjectData[1].transformation);

	//	// Apply object transformation
	//	DX::WorldBuffer world_buffer = {};
	//	world_buffer.world = DirectX::XMMatrixTranspose(matrix);
	//	world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
	//	world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());

	//	m_DxShader->UpdateWorldConstantBuffer(world_buffer);

	//	// Render object
	//	d3dDeviceContext->DrawIndexed(36, 3456, 576);
	//}
}
 
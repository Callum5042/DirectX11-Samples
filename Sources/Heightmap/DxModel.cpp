#include "DxModel.h"
#include <DirectXMath.h>
#include <vector>
#include "GeometryGenerator.h"
#include <fstream>
#include <cmath>

DX::Model::Model(DX::Renderer* renderer) : m_DxRenderer(renderer)
{
	World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
}

void DX::Model::Create()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	GeometryGenerator::CreateGrid(50.0f, 50.0f, 10, 10, this);

	CreateVertexBuffer();
	CreateIndexBuffer();

	// Load heightmap data and create shader resource view from it
	LoadHeightmap(d3dDevice);
}

void DX::Model::LoadHeightmap(ID3D11Device* d3dDevice)
{
	// Load heightmap
	std::ifstream file("..\\..\\Resources\\terrain\\heightmap_white.raw", std::fstream::in | std::fstream::binary);
	std::vector<float> heightmap_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Scale down
	for (size_t i = 0; i < heightmap_data.size(); i++)
	{
		heightmap_data[i] /= 8.0f;
	}

	// Assuming the heightmap is a square texture we can calculate size by sqrt
	static int heightmap_size = static_cast<int>(std::sqrt(heightmap_data.size()));

	// Smooth
	std::vector<float> dest(heightmap_data.size());
	for (int i = 0; i < heightmap_size; ++i)
	{
		for (int j = 0; j < heightmap_size; ++j)
		{
			dest[static_cast<size_t>(i * heightmap_size + j)] = Average(i, j, heightmap_data, heightmap_size);
		}
	}

	heightmap_data = dest;

	// Create Direct3D 11 texture from heightmap data
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width = heightmap_size;
	texture_desc.Height = heightmap_size;
	texture_desc.MipLevels = 1;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R32_FLOAT;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA subresource_data = {};
	subresource_data.pSysMem = heightmap_data.data();
	subresource_data.SysMemPitch = heightmap_size * sizeof(float);

	ComPtr<ID3D11Texture2D> heightmap_texture = nullptr;
	DX::Check(d3dDevice->CreateTexture2D(&texture_desc, &subresource_data, heightmap_texture.GetAddressOf()));

	// Build Direct3D 11 shader resource view from texture
	D3D11_SHADER_RESOURCE_VIEW_DESC resource_desc = {};
	resource_desc.Format = texture_desc.Format;
	resource_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resource_desc.Texture2D.MostDetailedMip = 0;
	resource_desc.Texture2D.MipLevels = -1;

	DX::Check(d3dDevice->CreateShaderResourceView(heightmap_texture.Get(), &resource_desc, m_HeightmapTexture.GetAddressOf()));
}

void DX::Model::CreateVertexBuffer()
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

void DX::Model::CreateIndexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

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

	// Bind the index buffer to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the pipeline's Input Assembler stage
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	// Bind heightmap texture to Vertex / Domain shader
	d3dDeviceContext->VSSetShaderResources(0, 1, m_HeightmapTexture.GetAddressOf());
	d3dDeviceContext->DSSetShaderResources(0, 1, m_HeightmapTexture.GetAddressOf());
	d3dDeviceContext->PSSetShaderResources(0, 1, m_HeightmapTexture.GetAddressOf());

	// Render geometry
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(Indices.size()), 0, 0);
}

float DX::Model::Average(int i, int j, std::vector<float>& heightmap_data, int heightmap_size)
{
	// Function computes the average height of the ij element.
	// It averages itself with its eight neighbor pixels.  
	// Note that if a pixel is missing neighbor, we just don't include it in the average. Edge pixels don't have a neighbor pixel.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// | 4| x| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	auto average = 0.0f;
	auto num = 0.0f;
	for (int m = i - 1; m <= i + 1; ++m)
	{
		for (int n = j - 1; n <= j + 1; ++n)
		{
			auto in_bounds = (m >= 0 && m < heightmap_size) && (n >= 0 && n < heightmap_size);

			if (in_bounds)
			{
				average += heightmap_data[static_cast<size_t>(m * heightmap_size + n)];
				num += 1.0f;
			}
		}
	}

	return average / num;
}
#include "DxModel.h"
#include <DirectXMath.h>
#include <vector>
#include "DDSTextureLoader.h"

constexpr uint32_t ConvertToInt(DirectX::XMVECTORF32 v)
{
	return ((int)(v.f[0] * 255.0f)) | ((int)(v.f[1] * 255.0f) << 8) | ((int)(v.f[2] * 255.0f) << 16) | ((int)(v.f[3] * 255.0f) << 24);
}

DX::Model::Model(DX::Renderer* renderer) : m_DxRenderer(renderer)
{
}

void DX::Model::Create()
{
	// Create input buffers
	CreateVertexBuffer();
	CreateIndexBuffer();

	// Create texture resource
	LoadTexture();
}

void DX::Model::CreateVertexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	const float width = 1.0f;
	const float height = 1.0f;
	const float depth = 1.0f;

	// Set vertex data
	std::vector<Vertex> vertices =
	{
		{ -width, -height, -depth, 0.0f, 1.0f },
		{ -width, +height, -depth, 0.0f, 0.0f },
		{ +width, +height, -depth, 1.0f, 0.0f },
		{ +width, -height, -depth, 1.0f, 1.0f },

		{ -width, -height, +depth, 1.0f, 1.0f },
		{ +width, -height, +depth, 0.0f, 1.0f },
		{ +width, +height, +depth, 0.0f, 0.0f },
		{ -width, +height, +depth, 1.0f, 0.0f },

		{ -width, +height, -depth, 0.0f, 1.0f },
		{ -width, +height, +depth, 0.0f, 0.0f },
		{ +width, +height, +depth, 1.0f, 0.0f },
		{ +width, +height, -depth, 1.0f, 1.0f },

		{ -width, -height, -depth, 1.0f, 1.0f },
		{ +width, -height, -depth, 0.0f, 1.0f },
		{ +width, -height, +depth, 0.0f, 0.0f },
		{ -width, -height, +depth, 1.0f, 0.0f },

		{ -width, -height, +depth, 0.0f, 1.0f },
		{ -width, +height, +depth, 0.0f, 0.0f },
		{ -width, +height, -depth, 1.0f, 0.0f },
		{ -width, -height, -depth, 1.0f, 1.0f },

		{ +width, -height, -depth, 0.0f, 1.0f },
		{ +width, +height, -depth, 0.0f, 0.0f },
		{ +width, +height, +depth, 1.0f, 0.0f },
		{ +width, -height, +depth, 1.0f, 1.0f }
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

void DX::Model::LoadTexture()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	/*ComPtr<ID3D11Resource> resource = nullptr;
	DX::Check(DirectX::CreateDDSTextureFromFile(d3dDevice, L"..\\..\\Resources\\Textures\\crate_diffuse.dds", 
		resource.ReleaseAndGetAddressOf(), m_DiffuseTexture.ReleaseAndGetAddressOf()));*/

	// Generate texture
	uint32_t resourceColour = ConvertToInt(DirectX::Colors::Green);

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &resourceColour;
	initData.SysMemPitch = sizeof(uint32_t);
	initData.SysMemSlicePitch = 0;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;


	ComPtr<ID3D11Texture2D> texture = nullptr;
	DX::Check(d3dDevice->CreateTexture2D(&desc, &initData, texture.GetAddressOf()));

	// Shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	DX::Check(d3dDevice->CreateShaderResourceView(texture.Get(), &SRVDesc, m_DiffuseTexture.GetAddressOf()));
}

void DX::Model::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// We need the stride and offset for the vertex
	UINT vertex_stride = sizeof(Vertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the Input Assembler
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// We need the stride and offset for the colour
	UINT colour_stride = sizeof(DirectX::XMVECTORF32);
	auto colour_offset = 0u;

	// Bind the index buffer to the Input Assembler
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the Input Assembler
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind texture to the pixel shader
	d3dDeviceContext->PSSetShaderResources(0, 1, m_DiffuseTexture.GetAddressOf());

	// Render geometry
	d3dDeviceContext->DrawIndexed(m_IndexCount, 0, 0);
}
 
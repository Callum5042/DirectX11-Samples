#include "DxSky.h"
#include <DirectXMath.h>
#include <vector>
#include "GeometryGenerator.h"

DX::Sky::Sky(DX::Renderer* renderer) : m_DxRenderer(renderer)
{
}

void DX::Sky::Create()
{
	GenerateSphere(2.0f, 8, 8);

	// Create input buffers
	CreateVertexBuffer();
	CreateIndexBuffer();

	// Create texture resource
	LoadTexture();

	// Create raster states
	CreateRasterState();

	// Create depth stencil
	CreateDepthStencilState();
}

void DX::Sky::CreateVertexBuffer()
{
	auto d3dDevice = m_DxRenderer->GetDevice();

	// Create index buffer
	D3D11_BUFFER_DESC vertex_buffer_desc = {};
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = static_cast<UINT>(sizeof(SkyVertex) * m_Vertices.size());
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertex_subdata = {};
	vertex_subdata.pSysMem = m_Vertices.data();

	DX::Check(d3dDevice->CreateBuffer(&vertex_buffer_desc, &vertex_subdata, m_d3dVertexBuffer.ReleaseAndGetAddressOf()));
}

void DX::Sky::CreateIndexBuffer()
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

void DX::Sky::LoadTexture()
{
	auto d3dDevice = m_DxRenderer->GetDevice();


}

void DX::Sky::CreateDepthStencilState()
{
	auto device = m_DxRenderer->GetDevice();

	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&depth_stencil_desc, &m_DepthStencilState);
}

void DX::Sky::CreateRasterState()
{
	auto device = m_DxRenderer->GetDevice();

	D3D11_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.AntialiasedLineEnable = true;
	rasterizerState.CullMode = D3D11_CULL_FRONT;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.FrontCounterClockwise = false;
	rasterizerState.MultisampleEnable = false;

	DX::Check(device->CreateRasterizerState(&rasterizerState, &m_RasterState));
}

void DX::Sky::GenerateSphere(float radius, UINT sliceCount, UINT stackCount)
{
	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	/*DX::Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f);
	DX::Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);*/

	DX::SkyVertex topVertex;
	topVertex.y = radius;

	DX::SkyVertex bottomVertex;
	bottomVertex.y = -radius;

	m_Vertices.push_back(topVertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (unsigned int i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (unsigned int j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			DX::SkyVertex v;

			// spherical to cartesian
			v.x = radius * sinf(phi) * cosf(theta);
			v.y = radius * cosf(phi);
			v.z = radius * sinf(phi) * sinf(theta);

			m_Vertices.push_back(v);
		}
	}

	m_Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (unsigned int i = 1; i <= sliceCount; ++i)
	{
		m_Indices.push_back(0);
		m_Indices.push_back(i + 1);
		m_Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	unsigned int baseIndex = 1;
	unsigned int ringVertexCount = sliceCount + 1;
	for (unsigned int i = 0; i < stackCount - 2; ++i)
	{
		for (unsigned int j = 0; j < sliceCount; ++j)
		{
			m_Indices.push_back(baseIndex + i * ringVertexCount + j);
			m_Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			m_Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			m_Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			m_Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			m_Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	unsigned int southPoleIndex = (unsigned int)m_Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (unsigned int i = 0; i < sliceCount; ++i)
	{
		m_Indices.push_back(southPoleIndex);
		m_Indices.push_back(baseIndex + i);
		m_Indices.push_back(baseIndex + i + 1);
	}
}

void DX::Sky::Render()
{
	auto d3dDeviceContext = m_DxRenderer->GetDeviceContext();

	// Set raster state
	d3dDeviceContext->RSSetState(m_RasterState.Get());

	// Set depth stencil
	d3dDeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 0);

	// We need the stride and offset for the vertex
	UINT vertex_stride = sizeof(SkyVertex);
	auto vertex_offset = 0u;

	// Bind the vertex buffer to the Input Assembler
	d3dDeviceContext->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &vertex_stride, &vertex_offset);

	// Bind the index buffer to the Input Assembler
	d3dDeviceContext->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Bind the geometry topology to the Input Assembler
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind texture to the pixel shader
	//d3dDeviceContext->PSSetShaderResources(0, 1, m_DiffuseTexture.GetAddressOf());

	// Render geometry
	d3dDeviceContext->DrawIndexed(static_cast<UINT>(m_Indices.size()), 0, 0);
}

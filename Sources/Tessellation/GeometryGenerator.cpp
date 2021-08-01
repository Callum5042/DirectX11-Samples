#include "GeometryGenerator.h"

void GeometryGenerator::CreateSphere(float radius, unsigned sliceCount, unsigned stackCount, DX::Model* model)
{
	model->Vertices.clear();
	model->Indices.clear();

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	DX::Vertex topVertex;
	topVertex.x = 0.0f;
	topVertex.y = radius;
	topVertex.z = 0.0f;
	topVertex.colour.r = 1.0f;

	DX::Vertex bottomVertex;
	bottomVertex.x = 0.0f;
	bottomVertex.y = -radius;
	bottomVertex.z = 0.0f;
	bottomVertex.colour.r = 1.0f;

	model->Vertices.push_back(topVertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (UINT i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			DX::Vertex v;

			// spherical to cartesian
			v.x = radius * sinf(phi) * cosf(theta);
			v.y = radius * cosf(phi);
			v.z = radius * sinf(phi) * sinf(theta);

			// Colour
			v.colour.r = 1.0f;
			v.colour.g = 0.0f;
			v.colour.b = 0.0f;
			v.colour.a = 1.0f;

			model->Vertices.push_back(v);
		}
	}

	model->Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (UINT i = 1; i <= sliceCount; ++i)
	{
		model->Indices.push_back(0);
		model->Indices.push_back(i + 1);
		model->Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			model->Indices.push_back(baseIndex + i * ringVertexCount + j);
			model->Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			model->Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			model->Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			model->Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			model->Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	UINT southPoleIndex = (UINT)model->Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		model->Indices.push_back(southPoleIndex);
		model->Indices.push_back(baseIndex + i);
		model->Indices.push_back(baseIndex + i + 1);
	}
}

void GeometryGenerator::CreateTriangle(float length, DX::Model* model)
{
	DX::Vertex v1;
	v1.x = 0.0f;
	v1.y = length;
	v1.z = 0.0f;
	v1.colour.r = 1.0f;

	DX::Vertex v2;
	v2.x = length;
	v2.y = -length;
	v2.z = 0.0f;
	v2.colour.r = 1.0f;

	DX::Vertex v3;
	v3.x = -length;
	v3.y = -length;
	v3.z = 0.0f;
	v3.colour.r = 1.0f;

	model->Vertices.push_back(v1);
	model->Vertices.push_back(v2);
	model->Vertices.push_back(v3);

	model->Indices.push_back(0);
	model->Indices.push_back(1);
	model->Indices.push_back(2);
}

void GeometryGenerator::CreateGrid(float width, float depth, UINT m, UINT n, DX::Model* model)
{
	UINT vertexCount = m * n;
	UINT faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	model->Vertices.resize(vertexCount);
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			int index = i * n + j;
			model->Vertices[index].x = x;
			model->Vertices[index].y = z;
			model->Vertices[index].z = 0.0f;
		}
	}

	//
	// Create the indices.
	//

	model->Indices.resize(static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(faceCount) * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	UINT k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			model->Indices[k] = i * n + j;
			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 1] = i * n + j + 1;
			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 2] = (i + 1) * n + j;

			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 3] = (i + 1) * n + j;
			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 4] = i * n + j + 1;
			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}
}

void GeometryGenerator::CreateRectangle(float width, float height, DX::Model* model)
{
	DX::Vertex v1;
	v1.x = -width;
	v1.y = height;
	v1.z = 0.0f;
	v1.colour.r = 0.0f;
	v1.colour.g = 1.0f;
	v1.colour.b = 0.0f;

	DX::Vertex v2;
	v2.x = width;
	v2.y = height;
	v2.z = 0.0f;
	v2.colour.r = 0.0f;
	v2.colour.g = 0.0f;
	v2.colour.b = 1.0f;

	DX::Vertex v3;
	v3.x = -width;
	v3.y = -height;
	v3.z = 0.0f;
	v3.colour.r = 1.0f;
	v3.colour.g = 1.0f;
	v3.colour.b = 0.0f;

	DX::Vertex v4;
	v4.x = width;
	v4.y = -height;
	v4.z = 0.0f;
	v4.colour.r = 1.0f;

	model->Vertices.push_back(v1);
	model->Vertices.push_back(v2);
	model->Vertices.push_back(v3);
	model->Vertices.push_back(v4);

	model->Indices.push_back(0);
	model->Indices.push_back(1);
	model->Indices.push_back(2);
	model->Indices.push_back(3);
}
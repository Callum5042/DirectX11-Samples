#include "GeometryGenerator.h"

void GeometryGenerator::CreateQuadGrid(float width, float depth, UINT m, UINT n, DX::Model* model)
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

			// Position
			model->Vertices[index].x = x;
			model->Vertices[index].y = 0.0f;
			model->Vertices[index].z = z;

			// Texture UV
			model->Vertices[i * n + j].u = j * du;
			model->Vertices[i * n + j].v = i * dv;
		}
	}

	//
	// Create the indices.
	//

	model->Indices.resize(static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(faceCount) * 4); // 3 indices per face

	// Iterate over each quad and compute indices.
	UINT k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			model->Indices[k] = i * n + j;
			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 1] = i * n + j + 1;

			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 2] = (i + 1) * n + j;
			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 3] = (i + 1) * n + j +1;

			k += 4; // next quad
		}
	}
}

void GeometryGenerator::CreateRectangle(float width, float height, DX::Model* model)
{
	DX::Vertex v1;
	v1.x = -width;
	v1.y = height;
	v1.z = 0.0f;
	v1.u = 0.0f;
	v1.v = 0.0f;

	DX::Vertex v2;
	v2.x = width;
	v2.y = height;
	v2.z = 0.0f;
	v2.u = 1.0f;
	v2.v = 0.0f;

	DX::Vertex v3;
	v3.x = -width;
	v3.y = -height;
	v3.z = 0.0f;
	v3.u = 0.0f;
	v3.v = 1.0f;

	DX::Vertex v4;
	v4.x = width;
	v4.y = -height;
	v4.z = 0.0f;
	v4.u = 1.0f;
	v4.v = 1.0f;

	model->Vertices.push_back(v1);
	model->Vertices.push_back(v2);
	model->Vertices.push_back(v3);
	model->Vertices.push_back(v4);

	model->Indices.push_back(0);
	model->Indices.push_back(1);
	model->Indices.push_back(2);

	model->Indices.push_back(2);
	model->Indices.push_back(1);
	model->Indices.push_back(3);
}
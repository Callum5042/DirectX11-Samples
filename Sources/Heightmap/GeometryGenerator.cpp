#include "GeometryGenerator.h"

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

			// Vertices
			model->Vertices[index].x = x;
			model->Vertices[index].y = 0.0f;
			model->Vertices[index].z = z;

			// Stretch texture over grid.
			model->Vertices[index].u = j * du;
			model->Vertices[index].v = i * dv;
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
			model->Indices[static_cast<std::vector<UINT, std::allocator<UINT>>::size_type>(k) + 3] = (i + 1) * n + j + 1;

			// next quad
			k += 4; 
		}
	}
}

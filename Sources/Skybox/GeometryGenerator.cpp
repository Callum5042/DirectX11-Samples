#include "GeometryGenerator.h"
#include <DirectXMath.h>

void Geometry::CreateBox(float width, float height, float depth, DX::MeshData* mesh)
{
	// Vertices
	std::vector<DX::Vertex> vertices =
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

	// Indices
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

	// Copy data
	mesh->vertices.assign(vertices.begin(), vertices.end());
	mesh->indices.assign(indices.begin(), indices.end());
}

void Geometry::CreateSphere(float radius, UINT sliceCount, UINT stackCount, DX::MeshData* meshData)
{
	meshData->vertices.clear();
	meshData->indices.clear();

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	/*DX::Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f);
	DX::Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);*/

	DX::Vertex topVertex;
	topVertex.y = radius;
	topVertex.u = 0.0f;
	topVertex.v = 0.0f;

	DX::Vertex bottomVertex;
	bottomVertex.y = -radius;
	bottomVertex.u = 0.0f;
	bottomVertex.v = 1.0f;

	meshData->vertices.push_back(topVertex);

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

			DX::Vertex v;

			// spherical to cartesian
			v.x = radius * sinf(phi) * cosf(theta);
			v.y = radius * cosf(phi);
			v.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			/*v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);*/

			/*DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&v.TangentU);
			DirectX::XMStoreFloat3(&v.TangentU, DirectX::XMVector3Normalize(T));*/

			// Normals
			/*DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3(v.x, v.y, v.z);
			DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&pos);

			DirectX::XMFLOAT3 normal;
			DirectX::XMStoreFloat3(&normal, DirectX::XMVector3Normalize(p));

			v.nx = normal.x;
			v.ny = normal.y;
			v.nz = normal.z;*/

			// Texture UV
			v.u = theta / DirectX::XM_2PI;
			v.v = phi / DirectX::XM_PI;

			meshData->vertices.push_back(v);
		}
	}

	meshData->vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (unsigned int i = 1; i <= sliceCount; ++i)
	{
		meshData->indices.push_back(0);
		meshData->indices.push_back(i + 1);
		meshData->indices.push_back(i);
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
			meshData->indices.push_back(baseIndex + i * ringVertexCount + j);
			meshData->indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData->indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData->indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData->indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData->indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	unsigned int southPoleIndex = (unsigned int)meshData->vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (unsigned int i = 0; i < sliceCount; ++i)
	{
		meshData->indices.push_back(southPoleIndex);
		meshData->indices.push_back(baseIndex + i);
		meshData->indices.push_back(baseIndex + i + 1);
	}
}

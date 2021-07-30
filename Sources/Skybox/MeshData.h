#pragma once

#include <vector>
typedef unsigned int UINT;

namespace DX
{
	// Vertex
	struct Vertex
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Texture UV
		float u = 0;
		float v = 0;
	};

	// Mesh
	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
	};
}
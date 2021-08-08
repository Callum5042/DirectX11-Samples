#pragma once

#include <vector>
typedef unsigned int UINT;

namespace DX
{
	struct Vertex
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Vertex UV's
		float u = 0;
		float v = 0;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
	};
}
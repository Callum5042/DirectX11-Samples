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

		// Vertex normals
		float nx = 0;
		float ny = 0;
		float nz = 0;
	};

	struct OverlayVertex
	{
		// Vertex position
		float x = 0;
		float y = 0;
		float z = 0;

		// Vertex texture mapping
		float u = 0;
		float v = 0;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
	};
}
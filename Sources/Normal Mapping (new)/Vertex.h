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

		// Vertex texture UV's
		float u = 0;
		float v = 0;

		// Vertex normals
		float nx = 0;
		float ny = 0;
		float nz = 0;

		// Vertex tangents
		float tx = 0;
		float ty = 0;
		float tz = 0;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;
	};
}
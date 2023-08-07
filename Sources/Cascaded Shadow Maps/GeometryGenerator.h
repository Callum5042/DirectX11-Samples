#pragma once

#include "Vertex.h"

namespace GeometryGenerator
{
	void CreateBox(float width, float height, float depth, DX::MeshData* meshData);

	void CreatePlane(float width, float depth, DX::MeshData* meshData);
}
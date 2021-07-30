#pragma once

#include "MeshData.h"

namespace Geometry
{
	void CreateBox(float width, float height, float depth, DX::MeshData* mesh);

	void CreateSphere(float radius, UINT sliceCount, UINT stackCount, DX::MeshData* meshData);
}
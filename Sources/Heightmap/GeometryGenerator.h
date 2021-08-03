#pragma once

#include "DxModel.h"

namespace GeometryGenerator
{
	void CreateSphere(float radius, unsigned sliceCount, unsigned stackCount, DX::Model* model);

	void CreateTriangle(float length, DX::Model* model);

	void CreateRectangle(float width, float height, DX::Model* model);

	void CreateGrid(float width, float depth, UINT m, UINT n, DX::Model* model);
}
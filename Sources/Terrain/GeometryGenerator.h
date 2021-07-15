#pragma once

#include "DxModel.h"

namespace GeometryGenerator
{
	void CreateQuadGrid(float width, float depth, UINT m, UINT n, DX::Model* model);

	void CreateRectangle(float width, float height, DX::Model* model);
}
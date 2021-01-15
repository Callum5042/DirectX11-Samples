#pragma once

#include "DxModel.h"

namespace GeometryGenerator
{
	void CreateSphere(float radius, unsigned sliceCount, unsigned stackCount, DX::Model* model);

	void CreateTriangle(float length, DX::Model* model);
}
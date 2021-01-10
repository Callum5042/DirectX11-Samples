#pragma once

#include <string>
#include "DxModel.h"

namespace ModelLoader
{
	bool Load(const std::string& path, DX::Mesh* mesh);
};
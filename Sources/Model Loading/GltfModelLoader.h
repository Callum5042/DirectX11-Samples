#pragma once

#include "DxModel.h"
#include <filesystem>
#include "simdjson.h"
using namespace simdjson;
using namespace simdjson::dom;

// glTF is a royalty-free 3D file format
// https://www.khronos.org/gltf/
// https://github.com/KhronosGroup/glTF
// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
// https://www.khronos.org/files/gltf20-reference-guide.pdf

struct GltfFileData
{
	std::vector<DX::Vertex> vertices;
	std::vector<UINT> indices;
	std::vector<DX::ModelObjectData> model_object_data;
};

class GltfModelLoader
{
public:
	GltfModelLoader() = default;
	virtual ~GltfModelLoader() = default;

	GltfFileData Load(const std::filesystem::path path);

private:
	GltfFileData m_Data;
	simdjson_result<element> m_Document;
};
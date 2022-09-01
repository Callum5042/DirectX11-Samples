#include "GltfModelLoader.h"
#include <fstream>

GltfFileData GltfModelLoader::Load(const std::filesystem::path path)
{
	m_Path = path;

	// Parser
	parser parser;
	m_Document = parser.load(path.string());

	// Loop through the nodes and check for nodes that contain a mesh index
	for (auto node : m_Document["nodes"])
	{
		// Only check for nodes that contain a mesh index
		auto mesh_index = node["mesh"].get_int64();
		if (mesh_index.error() != simdjson::SUCCESS)
			continue;

		// Read mesh
		auto mesh = m_Document["meshes"].at(mesh_index.value());

		// Mesh name
		auto mesh_name = mesh["name"].get_string();

		// Get first mesh primitive - we're only assuming we will ever have 1 mesh primitive in this situation
		auto mesh_primitives = mesh["primitives"].at(0);

		// Load mesh vertices
		auto mesh_vertex_position_index = mesh_primitives["attributes"]["POSITION"].get_int64();
		LoadVertices(mesh_vertex_position_index.value());

		// Load mesh indices
		auto mesh_indices_index = mesh_primitives["indices"].get_int64();
		LoadIndices(mesh_indices_index.value());
	}

	return m_Data;
}

void GltfModelLoader::LoadVertices(int64_t vertices_index)
{
	// Get vertex position accessors
	auto mesh_vertex_position_accessor = m_Document["accessors"].at(vertices_index);

	// Vertex count
	auto vertex_count = mesh_vertex_position_accessor["count"].get_int64();

	// Buffer
	auto vertex_buffer_view_index = mesh_vertex_position_accessor["bufferView"].get_int64();
	std::vector<char> buffer = LoadBuffer(vertex_buffer_view_index.value());

	// Reinterpret the data to what we set in the input layout
	DX::Vertex* raw_vertices = reinterpret_cast<DX::Vertex*>(buffer.data());
	std::vector<DX::Vertex> vertices(raw_vertices, raw_vertices + vertex_count.value());

	// Set vertices
	m_Data.vertices = vertices;
}

void GltfModelLoader::LoadIndices(int64_t indices_index)
{
	// Indices count
	auto mesh_indices_accessor = m_Document["accessors"].at(indices_index);
	auto indices_count = mesh_indices_accessor["count"].get_int64();

	// Buffer
	auto indices_buffer_view_index = mesh_indices_accessor["bufferView"].get_int64();
	std::vector<char> buffer = LoadBuffer(indices_buffer_view_index.value());

	// Reinterpret the data to what we set in the input layout
	USHORT* raw_indices = reinterpret_cast<USHORT*>(buffer.data());
	std::vector<UINT> indices(raw_indices, raw_indices + indices_count.value());
	m_Data.indices = indices;

	// To be moved
	DX::ModelObjectData model_object_data = {};
	model_object_data.index_count = indices_count.value();
	model_object_data.index_start = 0;
	m_Data.model_object_data.push_back(model_object_data);
}

std::vector<char> GltfModelLoader::LoadBuffer(int64_t index)
{
	// Buffer view data
	auto vertex_buffer_view = m_Document["bufferViews"].at(index);
	auto buffer_index = vertex_buffer_view["buffer"].get_int64();
	auto buffer_byte_length = vertex_buffer_view["byteLength"].get_int64();
	auto buffer_byte_offset = vertex_buffer_view["byteOffset"].get_int64();

	// Set buffer data
	auto buffer = m_Document["buffers"].at(buffer_index.value());
	auto buffer_length = buffer["byteLength"].get_int64();
	auto buffer_uri = buffer["uri"].get_string();

	// Read buffer
	auto buffer_file = m_Path.parent_path() / buffer_uri.value();
	std::ifstream file(buffer_file.string(), std::fstream::in | std::fstream::binary);
	file.seekg(buffer_byte_offset.value());

	std::vector<char> data;
	data.resize(buffer_byte_length.value());
	file.read(data.data(), buffer_byte_length.value());
	file.close();

	return data;
}

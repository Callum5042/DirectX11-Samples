#include "GltfModelLoader.h"
#include <fstream>

GltfFileData GltfModelLoader::Load(const std::filesystem::path path)
{
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

		// Get vertex position index
		{
			auto mesh_vertex_position_index = mesh_primitives["attributes"]["POSITION"].get_int64();

			// Get vertex position accessors
			auto mesh_vertex_position_accessor = m_Document["accessors"].at(mesh_vertex_position_index.value());

			// Vertex count
			auto vertex_count = mesh_vertex_position_accessor["count"].get_int64();

			// Vertex buffer view
			auto vertex_buffer_view_index = mesh_vertex_position_accessor["bufferView"].get_int64();

			auto vertex_buffer_view = m_Document["bufferViews"].at(vertex_buffer_view_index.value());
			auto buffer_index = vertex_buffer_view["buffer"].get_int64();
			auto buffer_byte_length = vertex_buffer_view["byteLength"].get_int64();
			auto buffer_byte_offset = vertex_buffer_view["byteOffset"].get_int64();

			// Load buffer
			auto buffer = m_Document["buffers"].at(buffer_index.value());
			auto buffer_length = buffer["byteLength"].get_int64();
			auto buffer_uri = buffer["uri"].get_string();

			// Buffer
			auto buffer_file = path.parent_path() / buffer_uri.value();
			std::ifstream file(buffer_file.string(), std::fstream::in | std::fstream::binary);
			file.seekg(buffer_byte_offset.value());

			std::vector<char> data;
			data.resize(buffer_byte_length.value());
			file.read(data.data(), buffer_byte_length.value());
			file.close();

			DX::Vertex* raw_vertices = reinterpret_cast<DX::Vertex*>(data.data());
			std::vector<DX::Vertex> vertices(raw_vertices, raw_vertices + vertex_count.value());
			m_Data.vertices = vertices;
		}

		//
		/// Mesh Index
		//

		// Get mesh indices index
		{
			auto mesh_indices_index = mesh_primitives["indices"].get_int64();

			auto mesh_indices_accessor = m_Document["accessors"].at(mesh_indices_index.value());

			// Indices count
			auto indices_count = mesh_indices_accessor["count"].get_int64();

			// Indices buffer view
			auto indices_buffer_view_index = mesh_indices_accessor["bufferView"].get_int64();

			auto vertex_buffer_view = m_Document["bufferViews"].at(indices_buffer_view_index.value());
			auto buffer_index = vertex_buffer_view["buffer"].get_int64();
			auto buffer_byte_length = vertex_buffer_view["byteLength"].get_int64();
			auto buffer_byte_offset = vertex_buffer_view["byteOffset"].get_int64();

			// Load buffer
			auto buffer = m_Document["buffers"].at(buffer_index.value());
			auto buffer_length = buffer["byteLength"].get_int64();
			auto buffer_uri = buffer["uri"].get_string();

			// Buffer
			auto buffer_file = path.parent_path() / buffer_uri.value();
			std::ifstream file(buffer_file.string(), std::fstream::in | std::fstream::binary);
			file.seekg(buffer_byte_offset.value());

			std::vector<char> data;
			data.resize(buffer_byte_length.value());
			file.read(data.data(), buffer_byte_length.value());
			file.close();

			USHORT* raw_indices = reinterpret_cast<USHORT*>(data.data());
			std::vector<UINT> indices(raw_indices, raw_indices + indices_count.value());
			m_Data.indices = indices;

			DX::ModelObjectData model_object_data;
			model_object_data.index_count = indices_count.value();
			model_object_data.index_start = 0;
			m_Data.model_object_data.push_back(model_object_data);
		}
	}

	return m_Data;
}

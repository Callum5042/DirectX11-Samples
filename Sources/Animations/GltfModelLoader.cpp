#include "GltfModelLoader.h"
#include <fstream>

GltfFileData GltfModelLoader::Load(const std::filesystem::path path)
{
	m_Path = path;

	// Parser
	parser parser;
	m_Document = parser.load(path.string());

	// Vertex total - want to keep track of this so we know the base vertex for each object when rendering
	UINT vertex_total = 0;

	// Indices count - we want to keep track of the indices for the object_data index_start variable
	UINT index_total = 0;

	// Loop through the nodes and check for nodes that contain a mesh index
	for (auto node : m_Document["nodes"])
	{
		// Only check for nodes that contain a mesh index
		auto mesh_index = node["mesh"].get_int64();
		if (mesh_index.error() != simdjson::SUCCESS)
			continue;

		// Model data
		DX::ModelObjectData object_data = {};

		// Load transformation
		auto transformation_matrix = LoadTransformation(node);
		object_data.transformation = transformation_matrix;

		// Read mesh
		auto mesh = m_Document["meshes"].at(mesh_index.value());

		// Mesh name
		auto mesh_name = mesh["name"].get_string();

		// Get first mesh primitive - we're only assuming we will ever have 1 mesh primitive in this situation
		auto mesh_primitives = mesh["primitives"].at(0);

		// Load mesh vertices
		auto mesh_vertex_position_index = mesh_primitives["attributes"]["POSITION"].get_int64();
		UINT vertex_count = LoadVertices(mesh_vertex_position_index.value());

		// Set base vertex
		object_data.base_vertex = vertex_total;

		// Increase total vertex
		vertex_total += vertex_count;

		// Load mesh indices
		auto mesh_indices_index = mesh_primitives["indices"].get_int64();
		UINT index_count = LoadIndices(mesh_indices_index.value());

		// Set index data
		object_data.index_start = index_total;
		object_data.index_count = index_count;

		// Increase total index so we know when the next object index start is
		index_total += index_count;

		// Store the model object data
		m_Data.model_object_data.push_back(object_data);
	}

	return m_Data;
}

UINT GltfModelLoader::LoadVertices(int64_t vertices_index)
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
	m_Data.vertices.insert(m_Data.vertices.end(), vertices.begin(), vertices.end());

	// Return vertex count for rendering
	return static_cast<UINT>(vertices.size());
}

UINT GltfModelLoader::LoadIndices(int64_t indices_index)
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

	// Set indices
	m_Data.indices.insert(m_Data.indices.end(), indices.begin(), indices.end());

	// Return indices count
	return static_cast<UINT>(indices.size());
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

DirectX::XMMATRIX GltfModelLoader::LoadTransformation(simdjson::dom::element& node)
{
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

	// Rotation - this must be done before position otherwise it will skew the maths
	simdjson_result<array> rotation = node["rotation"].get_array();
	if (rotation.error() == simdjson::SUCCESS)
	{
		float x = static_cast<float>(rotation.at(0).get_double().value());
		float y = static_cast<float>(rotation.at(1).get_double().value());
		float z = static_cast<float>(rotation.at(2).get_double().value());
		float w = static_cast<float>(rotation.at(3).get_double().value());
		world *= DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(x, y, z, w));
	}

	// Scale
	simdjson_result<array> scale = node["scale"].get_array();
	if (scale.error() == simdjson::SUCCESS)
	{
		float x = static_cast<float>(scale.at(0).get_double().value());
		float y = static_cast<float>(scale.at(1).get_double().value());
		float z = static_cast<float>(scale.at(2).get_double().value());
		world *= DirectX::XMMatrixScaling(x, y, z);
	}

	// Position
	simdjson_result<array> translation = node["translation"].get_array();
	if (translation.error() == simdjson::SUCCESS)
	{
		float x = static_cast<float>(translation.at(0).get_double().value());
		float y = static_cast<float>(translation.at(1).get_double().value());
		float z = static_cast<float>(translation.at(2).get_double().value());
		world *= DirectX::XMMatrixTranslation(x, y, z);
	}

	return world;
}

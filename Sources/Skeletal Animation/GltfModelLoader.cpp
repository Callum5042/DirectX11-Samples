#include "GltfModelLoader.h"
#include <fstream>
#include <map>

namespace
{
	struct Position
	{
		float x;
		float y;
		float z;
	};

	struct Joint
	{
		unsigned char x;
		unsigned char y;
		unsigned char z;
		unsigned char w;
	};

	struct Weight
	{
		float x;
		float y;
		float z;
		float w;
	};

	struct InverseBindMatrix
	{
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
	};

	struct Scale
	{
		float x;
		float y;
		float z;
	};

	struct Quaternion
	{
		float x;
		float y;
		float z;
		float w;
	};
}

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
		DX::Subset object_data = {};

		// Load transformation
		auto transformation_matrix = LoadTransformation(node);
		// object_data.transformation = transformation_matrix;

		// Read mesh
		auto mesh = m_Document["meshes"].at(mesh_index.value());

		// Mesh name
		auto mesh_name = mesh["name"].get_string();

		// Get first mesh primitive - we're only assuming we will ever have 1 mesh primitive in this situation
		auto mesh_primitives = mesh["primitives"].at(0);

		// Load mesh vertices
		UINT vertex_count = LoadVertices(mesh_primitives.value());

		// Set base vertex
		object_data.baseVertex = vertex_total;

		// Increase total vertex
		vertex_total += vertex_count;

		// Load mesh indices
		auto mesh_indices_index = mesh_primitives["indices"].get_int64();
		UINT index_count = LoadIndices(mesh_indices_index.value());

		// Set index data
		object_data.startIndex = index_total;
		object_data.totalIndex = index_count;

		// Increase total index so we know when the next object index start is
		index_total += index_count;

		// Load joint data
		auto skin_index = node["skin"].get_int64();
		auto bones = LoadSkin(skin_index.value());
		m_Data.bones = bones;

		// Store the model object data
		m_Data.model_object_data.push_back(object_data);
	}

	LoadAnimations();

	return m_Data;
}

UINT GltfModelLoader::LoadVertices(simdjson::dom::element& primitive)
{
	auto position_index = primitive["attributes"]["POSITION"].get_int64().value();
	auto joints_index = primitive["attributes"]["JOINTS_0"].get_int64().value();
	auto weights_index = primitive["attributes"]["WEIGHTS_0"].get_int64().value();

	// Get vertex position accessors
	auto position_accessor = m_Document["accessors"].at(position_index);
	auto joints_accessor = m_Document["accessors"].at(joints_index);
	auto weights_accessor = m_Document["accessors"].at(weights_index);

	// Vertex count
	auto vertex_count = static_cast<UINT>(position_accessor["count"].get_int64().value());

	// Buffer
	std::vector<char> position_buffer = LoadBuffer(position_accessor["bufferView"].get_int64().value());
	std::vector<char> joints_buffer = LoadBuffer(joints_accessor["bufferView"].get_int64().value());
	std::vector<char> weights_buffer = LoadBuffer(weights_accessor["bufferView"].get_int64().value());

	// Reinterpret the data to what we set in the input layout
	Position* position = reinterpret_cast<Position*>(position_buffer.data());
	Joint* joints = reinterpret_cast<Joint*>(joints_buffer.data());
	Weight* weights = reinterpret_cast<Weight*>(weights_buffer.data());

	// Set vertices
	for (UINT i = 0; i < vertex_count; ++i)
	{
		DX::Vertex vertex;

		// Position
		vertex.x = position[i].x;
		vertex.y = position[i].y;
		vertex.z = position[i].z;

		// Joint
		vertex.bone[0] = joints[i].x;
		vertex.bone[1] = joints[i].y;
		vertex.bone[2] = joints[i].z;
		vertex.bone[3] = joints[i].w;

		// Weight
		vertex.weight[0] = weights[i].x;
		vertex.weight[1] = weights[i].y;
		vertex.weight[2] = weights[i].z;
		vertex.weight[3] = weights[i].w;

		m_Data.vertices.push_back(vertex);
	}

	// Return vertex count for rendering
	return vertex_count;
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

std::vector<DX::BoneInfo> GltfModelLoader::LoadSkin(int64_t skin_index)
{
	std::vector<DX::BoneInfo> bones;

	// Load skin
	auto skin = m_Document["skins"].at(skin_index);

	// Skin name
	auto skin_name = skin["name"].get_string();

	// Inverse bind matrix?
	auto inverseBindMatrices_index = skin["inverseBindMatrices"].get_int64();

	// Invese bind matrix 
	auto accessor = m_Document["accessors"].at(inverseBindMatrices_index.value());
	auto componentType = accessor["componentType"].get_int64();
	auto count = accessor["count"].get_int64();
	auto type = accessor["type"].get_string();

	auto bufferView_index = accessor["bufferView"].get_int64();
	std::vector<char> buffer = LoadBuffer(bufferView_index.value());
	InverseBindMatrix* raw_inverseBindMatrix = reinterpret_cast<InverseBindMatrix*>(buffer.data());

	// List of joints
	int index_count = 0;
	auto joints = skin["joints"].get_array();
	for (auto it = joints.begin(); it != joints.end(); ++it)
	{
		DX::BoneInfo bone = {};

		// Load bone data from node
		auto index = (*it).get_int64();
		bone.bone_index = static_cast<int>(index.value());
		auto node = m_Document["nodes"].at(index.value());

		// Get bone name
		auto name = node["name"].get_string().value();

		// Bone children
		auto child_bones = node["children"].get_array();
		if (child_bones.error() == simdjson::SUCCESS)
		{
			for (auto bone_it = child_bones.begin(); bone_it != child_bones.end(); ++bone_it)
			{
				auto bone_index = static_cast<int>((*bone_it).get_int64().value());
				bone.children.push_back(bone_index);
			}
		}

		// See if we can load any transformations
		auto matrix = DirectX::XMMatrixIdentity();

		auto translation = node["translation"].get_array();
		if (translation.error() == simdjson::SUCCESS)
		{
			float x = static_cast<float>(translation.value().at(0).get_double());
			float y = static_cast<float>(translation.value().at(1).get_double());
			float z = static_cast<float>(translation.value().at(2).get_double());

			matrix *= DirectX::XMMatrixTranslation(x, y, z);
		}

		// Inverse bind
		auto m = raw_inverseBindMatrix[index_count];

		// Must tranpose it to DirectX row major
		DirectX::XMMATRIX ibm(m.m00, m.m10, m.m20, m.m30,
							  m.m01, m.m11, m.m21, m.m31,
							  m.m02, m.m12, m.m22, m.m32,
							  m.m03, m.m13, m.m23, m.m33);

		// Fill struct
		bone.name = name;
		bone.offset = ibm;

		bones.push_back(bone);

		index_count++;
	}

	// Set bone parent
	for (auto& bone : bones)
	{
		for (auto& child_index : bone.children)
		{
			auto child_bone = std::find_if(bones.begin(), bones.end(), [&](const DX::BoneInfo& bone)
			{
				return (bone.bone_index == child_index);
			});

			(*child_bone).parentName = bone.name;
		}
	}

	for (int i = 0; i < bones.size(); ++i)
	{
		int parentId = 0;
		for (int j = 0; j < bones.size(); ++j)
		{
			if (bones[i].parentName == bones[j].name)
			{
				parentId = j;
				break;
			}
		}

		bones[i].parentId = parentId;
	}

	return bones;
}

void GltfModelLoader::LoadAnimations()
{
	// Animations
	for (auto animation : m_Document["animations"])
	{
		auto name = animation["name"].get_string();
		auto channels = animation["channels"];
		auto samplers = animation["samplers"];

		std::map<int, std::vector<float>> times;
		std::map<int, std::vector<Position>> translations;
		std::map<int, std::vector<Scale>> scales;
		std::map<int, std::vector<Quaternion>> rotations;

		// Set channels
		for (auto channel_iterator = channels.begin(); channel_iterator != channels.end(); ++channel_iterator)
		{
			auto channel = (*channel_iterator);

			// Channel details
			auto bone_index = static_cast<int>(channel["target"]["node"].get_int64().value());
			auto path = channel["target"]["path"].get_string().value();
			auto sampler_index = channel["sampler"].get_int64();

			// Sampler details
			auto sampler = animation["samplers"].at(sampler_index.value());
			auto input_index = sampler["input"].get_int64();
			auto output_index = sampler["output"].get_int64();
			auto interpolation = sampler["interpolation"].get_string();

			// Input value
			auto input_accessor = m_Document["accessors"].at(input_index.value());
			auto input_accessor_count = input_accessor["count"].get_int64();
			std::vector<char> input_buffer = LoadBuffer(input_accessor["bufferView"].get_int64().value());

			float* raw_input_data = reinterpret_cast<float*>(input_buffer.data());
			std::vector<float> time_frame_data(raw_input_data, raw_input_data + input_accessor_count.value());
			times[bone_index] = time_frame_data;

			// Output
			auto output_accessor = m_Document["accessors"].at(output_index.value());
			auto output_accessor_count = output_accessor["count"].get_int64();
			std::vector<char> buffer = LoadBuffer(output_accessor["bufferView"].get_int64().value());

			if (path == "translation")
			{
				Position* raw_data = reinterpret_cast<Position*>(buffer.data());
				std::vector<Position> data(raw_data, raw_data + output_accessor_count.value());
				translations[bone_index] = data;
			}
			else if (path == "scale")
			{
				Scale* raw_data = reinterpret_cast<Scale*>(buffer.data());
				std::vector<Scale> data(raw_data, raw_data + output_accessor_count.value());
				scales[bone_index] = data;
			}
			else if (path == "rotation")
			{
				Quaternion* raw_data = reinterpret_cast<Quaternion*>(buffer.data());
				std::vector<Quaternion> data(raw_data, raw_data + output_accessor_count.value());
				rotations[bone_index] = data;
			}
		}

		// Turn data into framedata
		DX::AnimationClip clip;
		clip.BoneAnimations.resize(times.size());
		for (int i = 0; i < times.size(); ++i)
		{
			for (unsigned k = 0; k < times[i].size(); ++k)
			{
				auto time = times[i][k];
				auto position = translations[i][k];
				auto rotation = rotations[i][k];
				auto scale = scales[i][k];

				DX::Keyframe frame;
				frame.TimePos = static_cast<float>(time * 1000.0f);
				frame.Translation = DirectX::XMFLOAT3(position.x, position.y, position.z);
				frame.RotationQuat = DirectX::XMFLOAT4(rotation.x, rotation.y, rotation.z, rotation.w);
				frame.Scale = DirectX::XMFLOAT3(scale.x, scale.y, scale.z);

				clip.BoneAnimations[times.size() - i - 1].Keyframes.push_back(frame);
			}
		}

		m_Data.animationClip = clip;
	}
}
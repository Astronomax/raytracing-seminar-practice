#include <Scene.hpp>
#include <Gltf.hpp>
#include <fstream>
#include <filesystem>
#include <memory>

Distribution
build_distribution(const Scene &scene)
{
	std::vector<Distribution> primitive_distributions;
	for (const auto &primitive : scene.primitives) {
		if (primitive.material.emission.x > 0.f || primitive.material.emission.y > 0.f || primitive.material.emission.z > 0.f) {
			switch (primitive.type) {
				case (FigureType::BOX): {
					Distribution box(DistributionType::BOX);
					box.init_box(&primitive);
					primitive_distributions.push_back(std::move(box));
					break;
				}
				case (FigureType::ELLIPSOID): {
					Distribution ellipsoid(DistributionType::ELLIPSOID);
					ellipsoid.init_ellipsoid(&primitive);
					primitive_distributions.push_back(std::move(ellipsoid));
					break;
				}
				case (FigureType::TRIANGLE): {
					Distribution triangle(DistributionType::TRIANGLE);
					triangle.init_triangle(&primitive);
					primitive_distributions.push_back(std::move(triangle));
					break;
				}
				default:
					unreachable();
			}
		}
	}
	std::vector<Distribution> distributions;
	{
		Distribution cosine(DistributionType::COSINE);
		distributions.push_back(std::move(cosine));
	}
	if (!primitive_distributions.empty()) {
		Distribution mixed(DistributionType::MIXED_ON_PRIMITIVES);
		mixed.init_mixed_on_primitives(primitive_distributions);
		distributions.push_back(std::move(mixed));
	}
	{
		Distribution mixed(DistributionType::MIXED);
		mixed.init_mixed(std::move(distributions));
		return mixed;
	}
}

void
Scene::init() {
	distribution = build_distribution(*this);
	std::vector<const Primitive*> primitives_;
	primitives_.reserve(primitives.size());
	for(auto &primitive : primitives)
		primitives_.push_back(&primitive);
	bvh = BVH(primitives_);
}

void load_buffers(std::string_view gltf_file_name, const rapidjson::Document &gltfScene, Scene &scene) {
	const auto &buffer_specs = gltfScene["buffers"].GetArray();
	for (const auto &buffer_spec : buffer_specs) {
		size_t buffer_len = buffer_spec["byteLength"].GetUint();
		GltfBuffer buf(buffer_len);
		const auto gltf_file_path = std::filesystem::path(gltf_file_name);
		const auto buffer_file_path = gltf_file_path.parent_path().append(buffer_spec["uri"].GetString());
		std::ifstream bufferStream(buffer_file_path, std::ios::binary);
		bufferStream.read(buf.data(), (std::streamsize)buffer_len);
		scene.buffers.push_back(std::move(buf));
	}
}

void load_buffer_views(const rapidjson::Document &gltfScene, Scene &scene) {
	const auto &buffer_view_specs = gltfScene["bufferViews"].GetArray();
	for (const auto &buffer_view_spec : buffer_view_specs) {
		scene.bufferViews.push_back(GltfBufferView{
			buffer_view_spec["buffer"].GetUint(),
			buffer_view_spec["byteLength"].GetUint(),
			buffer_view_spec["byteOffset"].GetUint(),
		});
	}
}

void load_nodes(const rapidjson::Document &gltfScene, Scene &scene) {
	const auto &node_specs = gltfScene["nodes"].GetArray();
	for (const auto &node_spec : node_specs) {
		GltfNode node;
		if (node_spec.HasMember("mesh"))
			node.mesh = node_spec["mesh"].GetUint();
		if (node_spec.HasMember("camera"))
			node.camera = node_spec["camera"].GetUint();
		if (node_spec.HasMember("rotation")) {
			const auto &rotation = node_spec["rotation"].GetArray();
			node.rotation = {
				rotation[3].GetFloat(),
				rotation[0].GetFloat(),
				rotation[1].GetFloat(),
				rotation[2].GetFloat()
			};
		}
		if (node_spec.HasMember("translation")) {
			const auto &translation = node_spec["translation"].GetArray();
			node.translation = {
				translation[0].GetFloat(),
				translation[1].GetFloat(),
				translation[2].GetFloat()
			};
		}
		if (node_spec.HasMember("scale")) {
			const auto &scale = node_spec["scale"].GetArray();
			node.scale = {
				scale[0].GetFloat(),
				scale[1].GetFloat(),
				scale[2].GetFloat()
			};
		}
		if (node_spec.HasMember("children")) {
			const auto &children = node_spec["children"].GetArray();
			for (const auto &child : children)
				node.children.push_back(child.GetUint());
		}
		if (node_spec.HasMember("matrix")) {
			const auto &matrix = node_spec["matrix"].GetArray();
			float transition[4][4];
			for (size_t i = 0; i < 16; i++)
				transition[i % 4][i / 4] = matrix[i].GetFloat();
			node.transition = Transform(transition);
		}
		if (!node.transition.has_value())
			node.transition = Transform(node.translation, node.rotation, node.scale);
		scene.nodes.push_back(std::move(node));
	}
	for (size_t i = 0; i < scene.nodes.size(); i++)
		for (const auto &child : scene.nodes[i].children)
			scene.nodes[child].parent_node = i;
	for (auto &node : scene.nodes) {
		node.total_transition = node.transition.value();
		auto parent = node.parent_node;
		while (parent.has_value()) {
			node.total_transition = scene.nodes[parent.value()].transition.value().prod(
				node.total_transition);
			parent = scene.nodes[parent.value()].parent_node;
		}
	}
}

void load_meshes(const rapidjson::Document &gltfScene, Scene &scene) {
	const auto &mesh_specs = gltfScene["meshes"].GetArray();
	for (const auto &mesh_spec : mesh_specs) {
		GltfMesh mesh;
		for (const auto &primitive : mesh_spec["primitives"].GetArray()) {
			mesh.primitives.push_back(GltfPrimitive{
				primitive["attributes"]["POSITION"].GetUint(),
				primitive["indices"].GetUint(),
				primitive["material"].GetUint()
			});
		}
		scene.meshes.push_back(std::move(mesh));
	}
}

void load_accessors(const rapidjson::Document &gltfScene, Scene &scene) {
	const auto &accessor_specs = gltfScene["accessors"].GetArray();
	for (const auto &accessor_spec : accessor_specs) {
		auto accessor = GltfAccessor{
			accessor_spec["bufferView"].GetUint(),
			accessor_spec["count"].GetUint(),
			accessor_spec["componentType"].GetUint(),
			accessor_spec["type"].GetString(),
			0
		};
		if (accessor_spec.HasMember("byteOffset"))
			accessor.byte_offset = (size_t)accessor_spec["byteOffset"].GetFloat();
		scene.accessors.push_back(std::move(accessor));
	}
}

void load_materials(const rapidjson::Document &gltfScene, Scene &scene) {
	const auto &material_specs = gltfScene["materials"].GetArray();
	for (const auto &material_spec : material_specs) {
		GltfMaterial material;
		if (material_spec.HasMember("pbrMetallicRoughness")) {
			if (material_spec["pbrMetallicRoughness"].HasMember("baseColorFactor")) {
				auto color = material_spec["pbrMetallicRoughness"]["baseColorFactor"].GetArray();
				material.color = Color{
					color[0].GetFloat(),
					color[1].GetFloat(),
					color[2].GetFloat()
				};
				material.alpha = color[3].GetFloat();
			}
			if (material_spec["pbrMetallicRoughness"].HasMember("metallicFactor"))
				material.metallic_factor = material_spec["pbrMetallicRoughness"]["metallicFactor"].GetFloat();
		}
		if (material_spec.HasMember("emissiveFactor")) {
			const auto emission = material_spec["emissiveFactor"].GetArray();
			material.emission = Color{
				emission[0].GetFloat(),
				emission[1].GetFloat(),
				emission[2].GetFloat()
			};
		}
		if (material_spec.HasMember("extensions") && material_spec["extensions"].HasMember("KHR_materials_emissive_strength")) {
			float emission_factor = material_spec["extensions"]["KHR_materials_emissive_strength"]["emissiveStrength"].GetFloat();
			material.emission *= emission_factor;
		}
		if (material.alpha < 1)
			material.material = Material::DIELECTRIC;
		else if (material.metallic_factor > 0)
			material.material = Material::METALLIC;
		scene.materials.push_back(material);
	}
}

void load_primitives(Scene &scene) {
	for (const auto &node : scene.nodes) {
		if (!node.mesh.has_value())
			continue;
		const auto &mesh = node.mesh.value();
		for (const auto &gltf_primitive : scene.meshes[mesh].primitives) {
			std::vector<glm::vec3> positions;
			{
				const auto &accessor = scene.accessors[gltf_primitive.positions];
				const auto &buffer_view = scene.bufferViews[accessor.buffer_view];
				const auto &buffer = scene.buffers[buffer_view.buffer];
				size_t byte_offset = buffer_view.byte_offset + accessor.byte_offset;
				for (size_t i = 0; i < accessor.count; i++) {
					glm::vec3 position;
					position.x = *(reinterpret_cast<const float *>(buffer.data() + byte_offset +
										       12 * i));
					position.y = *(reinterpret_cast<const float *>(buffer.data() + byte_offset +
										       12 * i + 4));
					position.z = *(reinterpret_cast<const float *>(buffer.data() + byte_offset +
										       12 * i + 8));
					positions.push_back(position);
				}
			}
			{
				const auto &accessor = scene.accessors[gltf_primitive.indices];
				const auto &buffer_view = scene.bufferViews[accessor.buffer_view];
				const auto &buffer = scene.buffers[buffer_view.buffer];
				const auto &material = scene.materials[gltf_primitive.material];
				for (size_t i = 0; i < accessor.count; i += 3) {
					size_t pos1, pos2, pos3;
					if (accessor.component_type == 5123) {
						pos1 = *(reinterpret_cast<const uint16_t *>(buffer.data() +
											    buffer_view.byte_offset +
											    2 * i));
						pos2 = *(reinterpret_cast<const uint16_t *>(buffer.data() +
											    buffer_view.byte_offset +
											    2 * (i + 1)));
						pos3 = *(reinterpret_cast<const uint16_t *>(buffer.data() +
											    buffer_view.byte_offset +
											    2 * (i + 2)));
					} else {
						pos1 = *(reinterpret_cast<const uint32_t *>(buffer.data() +
											    buffer_view.byte_offset +
											    4 * i));
						pos2 = *(reinterpret_cast<const uint32_t *>(buffer.data() +
											    buffer_view.byte_offset +
											    4 * (i + 1)));
						pos3 = *(reinterpret_cast<const uint32_t *>(buffer.data() +
											    buffer_view.byte_offset +
											    4 * (i + 2)));
					}
					Primitive primitive;
					primitive.type = FigureType::TRIANGLE;
					primitive.primitive_specific[0] = node.total_transition.transform(
						positions[pos1]);
					primitive.primitive_specific[1] = node.total_transition.transform(
						positions[pos3]);
					primitive.primitive_specific[2] = node.total_transition.transform(
						positions[pos2]);
					primitive.material = material;
					scene.primitives.push_back(primitive);
				}
			}
		}
	}
}

void load_camera(const rapidjson::Document &gltfScene, Scene &scene) {
	scene.camera.up = {0, 1, 0};
	scene.camera.forward = {0, 0, -1};
	scene.camera.right = {1, 0, 0};

	for (const auto &node : scene.nodes) {
		if (node.camera.has_value()) {
			scene.camera.fov_y = gltfScene["cameras"].GetArray()[node.camera.value()]["perspective"]["yfov"].GetFloat();
			scene.camera.position = node.total_transition.transform({0, 0, 0});
			scene.camera.up = node.total_transition.transform(scene.camera.up) - scene.camera.position;
			scene.camera.right = node.total_transition.transform(scene.camera.right) - scene.camera.position;
			scene.camera.forward =
				node.total_transition.transform(scene.camera.forward) - scene.camera.position;
		}
	}
}

Scene load_scene(std::string_view gltfFilename) {
	Scene scene;

	std::ifstream in(gltfFilename.data(), std::ios_base::binary);
	rapidjson::IStreamWrapper isw(in);
	rapidjson::Document gltfScene;
	gltfScene.ParseStream(isw);
	load_buffers(gltfFilename, gltfScene, scene);
	load_buffer_views(gltfScene, scene);
	load_nodes(gltfScene, scene);
	load_meshes(gltfScene, scene);
	load_accessors(gltfScene, scene);
	load_materials(gltfScene, scene);
	load_primitives(scene);
	load_camera(gltfScene, scene);

	scene.init();
	return scene;
}
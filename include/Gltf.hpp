#ifndef RAYTRACING_GLTF_HPP
#define RAYTRACING_GLTF_HPP


#include <Primitive.hpp>
#include <Transform.hpp>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <cstddef>
#include <optional>
#include <vector>

using GltfBuffer = std::vector<char>;

struct GltfBufferView {
    size_t buffer;
    size_t byte_length;
    size_t byte_offset;
};

struct GltfAccessor {
    std::size_t buffer_view;
    std::size_t count;
    std::size_t component_type;
    std::string type;
    std::size_t byte_offset;
};

struct GltfNode {
    std::optional<std::size_t> mesh = {};
    std::optional<std::size_t> camera = {};
    glm::quat rotation = {1.f, 0.f, 0.f, 0.f};
    glm::vec3 scale = {1, 1, 1};
    glm::vec3 translation = {0, 0, 0};;
    std::optional<Transform> transition = {};
    std::vector<size_t> children;
    std::optional<size_t> parent_node = {};
    Transform total_transition;
};

struct GltfMaterial {
    Color color = {1, 1, 1};
    float alpha = 1.0;
    Color emission = {0, 0, 0};
    float metallic_factor = 1.0;
    Material material = Material::DIFFUSE;
    static constexpr const float ior = 1.5;
};

struct GltfPrimitive {
    std::size_t positions;
    std::size_t indices;
    std::size_t material;
};

struct GltfMesh {
    std::vector<GltfPrimitive> primitives;
};

#endif //RAYTRACING_GLTF_HPP

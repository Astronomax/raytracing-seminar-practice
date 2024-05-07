#ifndef RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

#include "BVH.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "Primitive.hpp"
#include "Gltf.hpp"
#include "Random.hpp"

#include <memory>
#include <vector>

struct Scene {
	void init();

	std::vector<GltfBuffer> buffers;
	std::vector<GltfBufferView> bufferViews;
	std::vector<GltfNode> nodes;
	std::vector<GltfMesh> meshes;
	std::vector<GltfAccessor> accessors;
	std::vector<GltfMaterial> materials;

	Camera camera;
	Color bg_color = black;
	BVH bvh;
	std::vector<Primitive> primitives;
	std::vector<Primitive> planes;
	int ray_depth = 1;
	int samples;
	Color ambient;
	Distribution distribution;
};

Scene load_scene(std::string_view gltfFilename);

#endif //RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

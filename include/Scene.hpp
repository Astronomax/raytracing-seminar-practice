#ifndef RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

#include "BVH.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "Primitive.hpp"
#include "Random.hpp"

#include <memory>
#include <vector>

struct Scene {
	void init(const std::shared_ptr<Random>& rnd);

	Camera camera;
	Color bg_color = black;
	BVH bvh;
	std::vector<PrimitivePtr> primitives;
	std::vector<PrimitivePtr> planes;
	int ray_depth = 1;
	int samples;
	Color ambient;
	std::unique_ptr<Distribution> distribution;
};


#endif //RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

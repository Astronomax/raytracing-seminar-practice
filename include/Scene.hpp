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
	void init();

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


#endif //RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

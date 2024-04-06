#ifndef RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

#include "Camera.hpp"
#include "Color.hpp"
#include "Primitive.hpp"

#include <vector>
#include <memory>

struct Scene {
	Camera camera;
	Color bg_color = black;
	std::vector<std::shared_ptr<Primitive>> primitives;
	int ray_depth = 1;
	int samples;
	Color ambient;
};


#endif //RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

#ifndef RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

#include "Camera.hpp"
#include "Color.hpp"
#include "Primitive.hpp"
#include "Light.hpp"

#include <vector>
#include <memory>

struct Scene {
	Camera camera;
	Color bg_color = black;
	std::vector<std::shared_ptr<Primitive>> primitives;
	int ray_depth = 1;
	Color ambient;
	std::vector<std::unique_ptr<Light>> lights;
};


#endif //RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

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
	std::vector<std::unique_ptr<Primitive>> primitives;
};


#endif //RAYTRACING_SEMINAR_PRACTICE_SCENE_HPP

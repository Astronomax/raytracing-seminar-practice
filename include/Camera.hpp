#ifndef RAYTRACING_SEMINAR_PRACTICE_CAMERA_HPP
#define RAYTRACING_SEMINAR_PRACTICE_CAMERA_HPP

#include "Ray.hpp"

#include <glm/vec3.hpp>

struct Camera {
	Ray ray_throw(float x, float y) const;

	int width, height;
	glm::vec3 position;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 forward;
	float tan_fov_x;
	float tan_fov_y;
};


#endif //RAYTRACING_SEMINAR_PRACTICE_CAMERA_HPP

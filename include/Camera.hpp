#ifndef RAYTRACING_SEMINAR_PRACTICE_CAMERA_HPP
#define RAYTRACING_SEMINAR_PRACTICE_CAMERA_HPP

#include "Ray.hpp"

#include <glm/vec3.hpp>

struct Camera {
	Ray ray_throw_pixel(int i, int j) const;

	int width, height;
	glm::vec3 position;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 forward;
	float fov_x;
	float fov_y;
};


#endif //RAYTRACING_SEMINAR_PRACTICE_CAMERA_HPP

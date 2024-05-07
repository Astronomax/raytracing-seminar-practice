#ifndef RAYTRACING_SEMINAR_PRACTICE_RAY_HPP
#define RAYTRACING_SEMINAR_PRACTICE_RAY_HPP

#include "glm/vec3.hpp"
#include "glm/glm.hpp"

struct Ray {
	glm::vec3 direction;
	glm::vec3 origin;
};

inline glm::vec3
walk_along(Ray ray, float distance)
{
	return ray.origin + glm::normalize(ray.direction) * distance;
}

#endif //RAYTRACING_SEMINAR_PRACTICE_RAY_HPP

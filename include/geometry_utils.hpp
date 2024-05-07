#ifndef RAYTRACING_GEOMETRY_UTILS_HPP
#define RAYTRACING_GEOMETRY_UTILS_HPP

#include "glm/vec3.hpp"
#include <glm/gtc/quaternion.hpp>

static const float PI = (float)acos(-1.0);

inline glm::vec3
rotate(glm::vec3 v, glm::quat q)
{
	glm::quat res = (conjugate(q) * glm::quat{0.f, v.x, v.y, v.z} * q);
	return {res.x, res.y, res.z};
}

#endif //RAYTRACING_GEOMETRY_UTILS_HPP

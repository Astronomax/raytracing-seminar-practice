#include "geometry_utils.hpp"

glm::vec3 rotate(glm::vec3 v, glm::quat q) {
	glm::quat res = (conjugate(q) * glm::quat{0.f, v.x, v.y, v.z} * q);
	return {res.x, res.y, res.z};
}
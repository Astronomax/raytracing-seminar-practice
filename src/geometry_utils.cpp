#include "geometry_utils.hpp"

glm::vec3 rotate(glm::vec3 v, glm::quat q) {
	glm::quat res = (glm::inverse(q) * glm::quat(0, v) * q);
	return {res.x, res.y, res.z};
}
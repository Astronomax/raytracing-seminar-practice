#include "Ray.hpp"

glm::vec3
walk_along(Ray ray, float distance)
{
	return ray.origin + glm::normalize(ray.direction) * distance;
}
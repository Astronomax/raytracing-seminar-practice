#ifndef RAYTRACING_GEOMETRY_UTILS_HPP
#define RAYTRACING_GEOMETRY_UTILS_HPP

#include "glm/vec3.hpp"
#include <glm/gtc/quaternion.hpp>

static const float PI = (float)acos(-1.0);
static const float EPS5 = 1e-5;
static const float INF = 1e18;

glm::vec3 rotate(glm::vec3 v, glm::quat q);

#endif //RAYTRACING_GEOMETRY_UTILS_HPP

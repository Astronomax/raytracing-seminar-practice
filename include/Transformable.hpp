#ifndef RAYTRACING_SEMINAR_PRACTICE_TRANSFORMABLE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_TRANSFORMABLE_HPP

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transformable {
	glm::vec3 position = {0, 0, 0};
	glm::quat rotation = {1, 0, 0, 0};
};

#endif //RAYTRACING_SEMINAR_PRACTICE_TRANSFORMABLE_HPP

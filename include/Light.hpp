#ifndef RAYTRACING_LIGHT_HPP
#define RAYTRACING_LIGHT_HPP

#include "Color.hpp"

struct Light {
	Color intensity;
	std::optional<glm::vec3> direction;
	glm::vec3 position;
	glm::vec3 attenuation;
};


#endif //RAYTRACING_LIGHT_HPP

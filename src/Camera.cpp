#include <Camera.hpp>
#include <geometry_utils.hpp>
#include <utils.hpp>
#include <glm/glm.hpp>

#include <cmath>

Ray
Camera::ray_throw(float x, float y) const
{
	assert(0.f <= x && x <= height + EPS5);
	assert(0.f <= y && y <= width + EPS5);
	return {
		glm::normalize(up * -((2.f * (float)x + 1.f) / (float)height - 1.f) * tanf(fov_y * 0.5f) +
					right * ((2.f * (float)y + 1.f) / (float)width - 1.f) * tanf(fov_x * 0.5f) + forward),
		position
	};
}
#include <Camera.hpp>

#include <geometry_utils.hpp>
#include <utils.hpp>

#include <glm/glm.hpp>

#include <cmath>

Ray
Camera::ray_throw(float x, float y) const
{
	assert(0.f <= x && x <= width + EPS5);
	assert(0.f <= y && y <= height + EPS5);

	return {
		glm::normalize(up * -((2.f * (float)y) / (float)height - 1.f) * tan_fov_y +
			       right * ((2.f * (float)x) / (float)width - 1.f) * tan_fov_x + forward),
		position
	};
}
#include "Camera.hpp"

#include <glm/glm.hpp>

#include <cmath>

Ray Camera::ray_throw_pixel(int row, int col) const {
	assert(0 <= row && row < height);
	assert(0 <= col && col < width);
	return {
		glm::normalize(\
		up * -((2.f * (float)row + 1.f) / (float)height - 1.f) * tanf(fov_y * 0.5f) + \
		right * ((2.f * (float)col + 1.f) / (float)width - 1.f) * tanf(fov_x * 0.5f) + \
		forward),\
		position
	};
}
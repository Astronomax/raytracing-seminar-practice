#ifndef RAYTRACING_SEMINAR_PRACTICE_IMAGE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_IMAGE_HPP

#include "Color.hpp"

#include <fstream>
#include <vector>

struct Image {
	Image(int height, int width);
	void set_pixel(int i, int j, Color color);
	void save(std::ofstream &out);

	int m_width, m_height;
	std::vector<uint8_t> raw;
};

#endif //RAYTRACING_SEMINAR_PRACTICE_IMAGE_HPP

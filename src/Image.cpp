#include "Image.hpp"

#include <cmath>

Image::Image(int height, int width) :
	m_height(height),
	m_width(width),
	raw(height * width * 3) {}

void Image::set_pixel(int i, int j, Color color) {
	assert(0 <= i && i < m_height);
	assert(0 <= j && j < m_width);
	auto ind = 3 * (i * m_width + j);
	assert(0.f <= color.x && color.x <= 1.f);
	assert(0.f <= color.y && color.y <= 1.f);
	assert(0.f <= color.z && color.z <= 1.f);
	raw[ind + 0] = (uint8_t)round(255.0 * color.x);
	raw[ind + 1] = (uint8_t)round(255.0 * color.y);
	raw[ind + 2] = (uint8_t)round(255.0 * color.z);
}

void Image::save(std::ofstream &out) {
	out << "P6\n";
	out << m_width << ' ' << m_height << '\n';
	out << 255 << '\n';
	out.write((char*)raw.data(), 3 * m_height * m_width);
}

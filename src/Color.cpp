#include "Color.hpp"

#include <cmath>

Color gamma_corrected(const Color &x) {
	auto gamma = 1.f / 2.2f;
	return {
		pow(x.x, gamma),
		pow(x.y, gamma),
		pow(x.z, gamma)
	};
}

static Color saturate(const Color &color) {
	auto x = std::min(1.f, std::max(0.f, color.x));
	auto y = std::min(1.f, std::max(0.f, color.y));
	auto z = std::min(1.f, std::max(0.f, color.z));
	return {x, y, z};
}

Color aces_tonemap(const Color &x) {
	const auto a = 2.51f;
	const auto b = 0.03f;
	const auto c = 2.43f;
	const auto d = 0.59f;
	const auto e = 0.14f;
	return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}
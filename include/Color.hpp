#ifndef RAYTRACING_SEMINAR_PRACTICE_COLOR_HPP
#define RAYTRACING_SEMINAR_PRACTICE_COLOR_HPP

#include <glm/vec3.hpp>

using Color = glm::vec3;

static auto black = glm::vec3(0, 0, 0);

Color gamma_corrected(const Color &x);
static Color saturate(const Color &color);
Color aces_tonemap(const Color &x);

#endif //RAYTRACING_SEMINAR_PRACTICE_COLOR_HPP

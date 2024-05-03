#ifndef RAYTRACING_SEMINAR_PRACTICE_RENDER_HPP
#define RAYTRACING_SEMINAR_PRACTICE_RENDER_HPP

#include "Image.hpp"
#include "Scene.hpp"

Image render(std::shared_ptr<Random> rnd, Scene &scene);

#endif //RAYTRACING_SEMINAR_PRACTICE_RENDER_HPP

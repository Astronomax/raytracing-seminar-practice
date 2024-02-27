#include "render.hpp"
#include "Ray.hpp"

Color raytrace(const Scene &scene, Ray ray) {
	Color obstacle_color = scene.bg_color;
	float min_distance = 1e12;
	for (auto &primitive : scene.primitives) {
		auto distance_opt = primitive->intersect(ray);
		if (!distance_opt.has_value())
			continue;
		auto distance = distance_opt.value();
		if (1e-7 < distance && distance < min_distance) {
			min_distance = distance;
			obstacle_color = primitive->color;
		}
	}
	return obstacle_color;
}

Image render(const Scene &scene) {
	const auto &camera = scene.camera;
	Image image(camera.height, camera.width);
	for (int i = 0; i < camera.height; i++) {
		for (int j = 0; j < camera.width; j++) {
			Ray ray = camera.ray_throw_pixel(i, j);
			image.set_pixel(i, j, raytrace(scene, ray));
		}
	}
	return image;
}
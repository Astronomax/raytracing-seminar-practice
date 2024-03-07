#include <iostream>
#include "render.hpp"
#include "Ray.hpp"

bool
intersect(const Scene &scene, Ray ray,
		  Intersection &intersection,
		  std::shared_ptr<Primitive> &obstacle)
{
	bool has_intersection = false;
	float min_distance = 1e12;
	for (auto &primitive : scene.primitives) {
		auto intersection_opt = primitive->intersect(ray);
		if (!intersection_opt.has_value())
			continue;
		float distance = intersection_opt.value().distance;
		if (1e-7 < distance && distance < min_distance) {
			min_distance = distance;
			intersection = intersection_opt.value();
			obstacle = primitive;
			has_intersection = true;
		}
	}
	return has_intersection;
}

Color raytrace(const Scene &scene, Ray ray, int depth = 0) {
	if (depth >= scene.ray_depth)
		return glm::vec3(0.f);

	Intersection intersection{};
	std::shared_ptr<Primitive> obstacle;
	if (!intersect(scene, ray, intersection, obstacle))
		return scene.bg_color;

	auto color = obstacle->color;
	Color light_color;
	auto point = intersection.point;
	auto normal = intersection.normal;
	assert(std::abs(glm::length(normal) - 1.f) < 1e-4f);
	assert(std::abs(glm::length(ray.direction) - 1.f) < 1e-4f);

	switch (obstacle->material) {
		case (Material::DIFFUSE): {
			light_color = scene.ambient;
			for (auto &light : scene.lights) {
				glm::vec3 light_direction;
				Color light_contribution;
				bool directed = light->direction.has_value();
				if (directed) {
					light_direction = light->direction.value();
					light_contribution = light->intensity;
				} else {
					light_direction = light->position - point;
					float r = glm::length(light_direction);
					light_contribution = light->intensity /
							glm::dot(light->attenuation,
									 glm::vec3(1.f, r, r * r));
				}
				light_direction = glm::normalize(light_direction);
				auto reflection = glm::dot(normal, light_direction);
				if (reflection < 0.f)
					continue;
				auto has_intersection = intersect(scene,
					{light_direction, point + light_direction * 1e-4f},
						intersection, obstacle);
				if (has_intersection) {
					if (directed || glm::length(light->position - point) >
									intersection.distance)
						continue;
				}
				light_color += reflection * light_contribution;
			}
			return light_color * color;
		}
		case (Material::METALLIC): {
			auto reflected_direction = ray.direction - 2.f * normal * glm::dot(normal, ray.direction);
			light_color = raytrace(scene,
				{reflected_direction, point + reflected_direction * 1e-4f}, depth + 1);
			return light_color * color;
		}
		case (Material::DIELECTRIC): {
			auto reflected_direction = ray.direction - 2.f * normal * glm::dot(normal, ray.direction);
			auto reflected = raytrace(scene,
				{reflected_direction, point + reflected_direction * 1e-4f}, depth + 1);
			auto eta1 = 1.f, eta2 = obstacle->ior;
			if (intersection.inside)
				std::swap(eta1, eta2);
			auto l = -1.f * ray.direction;
			auto cosTheta1 = glm::dot(normal, l);
			auto sinTheta1 = sqrtf(1.f - powf(cosTheta1, 2.f));
			auto sinTheta2 = eta1 / eta2 * sinTheta1;
			if (std::abs(sinTheta2) > 1.f)
				return reflected;
			auto cosTheta2 = sqrtf(1.f - powf(sinTheta2, 2.f));
			auto refracted_direction = eta1 / eta2 * (-1.f * l) +
					(eta1 / eta2 * glm::dot(normal, l) - cosTheta2) * normal;
			auto refracted = raytrace(scene,
				{refracted_direction, point + refracted_direction * 1e-4f}, depth + 1);
			if (!intersection.inside)
				refracted *= color;
			auto r0 = powf((eta1 - eta2) / (eta1 + eta2), 2.f);
			auto r = r0 + (1.f - r0) * powf(1.f - glm::dot(normal, l), 5.f);
			return r * reflected + (1.f - r) * refracted;
		}
		default: {
			// unreachable
			break;
		}
	}
}

Image render(const Scene &scene) {
	const auto &camera = scene.camera;
	Image image(camera.height, camera.width);
	for (int i = 0; i < camera.height; i++) {
		for (int j = 0; j < camera.width; j++) {
			auto ray = camera.ray_throw_pixel(i, j);
			image.set_pixel(i, j, gamma_corrected(aces_tonemap(raytrace(scene, ray))));
		}
	}
	return image;
}
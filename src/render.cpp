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

Color raytrace(const Scene &scene, Ray ray, int depth = 0);

Color light_contribution(const std::unique_ptr<Light> &light, glm::vec3 point) {
	if (light->direction.has_value()) {
		return light->intensity;
	} else {
		Color res = light->intensity;
		float r = glm::length(light->position - point);
		res /= glm::dot(light->attenuation, glm::vec3(1.f, r, r * r));
		return res;
	}
}

Color diffuse_raytrace(const Scene &scene, const std::shared_ptr<Primitive>& primitive,
		       glm::vec3 point, glm::vec3 normal)
{
	Intersection intersection{};
	std::shared_ptr<Primitive> obstacle;
	Color light_color = scene.ambient;
	for (auto &light : scene.lights) {
		glm::vec3 light_direction;
		float r = 1e18;
		if (light->direction.has_value()) {
			light_direction = light->direction.value();
		} else {
			light_direction = light->position - point;
			r = glm::length(light_direction);
		}
		light_direction = glm::normalize(light_direction);
		auto reflection = glm::dot(normal, light_direction);
		if (reflection < 0.f)
			continue;
		auto has_intersection = intersect(scene,
			{light_direction, point + light_direction * 1e-4f},
			intersection, obstacle);
		if (has_intersection && intersection.distance < r)
			continue;
		light_color += reflection * light_contribution(light, point);
	}
	return light_color * primitive->color;
}

Color metallic_raytrace(const Scene &scene, const std::shared_ptr<Primitive>& primitive,
			glm::vec3 point, glm::vec3 normal, Ray ray, int depth)
{
	auto reflect_dir = ray.direction - 2.f * normal * glm::dot(normal, ray.direction);
	return raytrace(scene, {reflect_dir, point + reflect_dir * 1e-4f}, depth + 1) * primitive->color;
}

Color dielectric_raytrace(const Scene &scene, const std::shared_ptr<Primitive>& primitive,
			  glm::vec3 point, glm::vec3 normal, Ray ray, bool inside, int depth)
{
	auto normal_ray_dot = glm::dot(normal, ray.direction);
	auto reflected_direction = ray.direction - 2.f * normal_ray_dot * normal;
	auto reflected = raytrace(scene, {reflected_direction, point + reflected_direction * 1e-4f}, depth + 1);
	auto eta1 = 1.f, eta2 = primitive->ior;
	if (inside)
		std::swap(eta1, eta2);
	auto cosTheta1 = -normal_ray_dot;
	auto sinTheta1 = sqrtf(1.f - powf(cosTheta1, 2.f));
	auto sinTheta2 = eta1 / eta2 * sinTheta1;
	if (std::abs(sinTheta2) > 1.f)
		return reflected;
	auto cosTheta2 = sqrtf(1.f - powf(sinTheta2, 2.f));
	auto refracted_direction = eta1 / eta2 * (ray.direction) + (eta1 / eta2 * cosTheta1 - cosTheta2) * normal;
	auto refracted = raytrace(scene, {refracted_direction, point + refracted_direction * 1e-4f}, depth + 1);
	if (!inside) {
		refracted *= primitive->color;
	}
	auto r0 = powf((eta1 - eta2) / (eta1 + eta2), 2.f);
	auto r = r0 + (1.f - r0) * powf(1.f + normal_ray_dot, 5.f);
	return r * reflected + (1.f - r) * refracted;
}

Color raytrace(const Scene &scene, Ray ray, int depth) {
	if (depth >= scene.ray_depth)
		return black;

	glm::vec3 point, normal;
	bool inside;
	std::shared_ptr<Primitive> obstacle;
	{
		Intersection intersection{};
		if (!intersect(scene, ray, intersection, obstacle))
			return scene.bg_color;
		point = intersection.point;
		normal = intersection.normal;
		inside = intersection.inside;
	}

	assert(std::abs(glm::length(normal) - 1.f) < 1e-5f);
	assert(std::abs(glm::length(ray.direction) - 1.f) < 1e-5f);

	switch (obstacle->material) {
		case (Material::DIFFUSE): {
			return diffuse_raytrace(scene, obstacle, point, normal);
		}
		case (Material::METALLIC): {
			return metallic_raytrace(scene, obstacle, point, normal, ray, depth + 1);
		}
		case (Material::DIELECTRIC): {
			return dielectric_raytrace(scene, obstacle, point, normal, ray, inside, depth);
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
#include <iostream>
#include "render.hpp"
#include "Ray.hpp"
#include "geometry_utils.hpp"
#include "utils.hpp"
#include "Random.hpp"

bool
intersect(const Scene &scene, Ray ray,
		  Intersection &intersection,
		  std::shared_ptr<Primitive> &obstacle)
{
	bool has_intersection = false;
	float min_distance = INF;
	for (auto &primitive : scene.primitives) {
		auto intersection_opt = primitive->intersect(ray);
		if (!intersection_opt.has_value())
			continue;
		float distance = intersection_opt.value().distance;
		if (EPS5 < distance && distance < min_distance) {
			min_distance = distance;
			intersection = intersection_opt.value();
			obstacle = primitive;
			has_intersection = true;
		}
	}
	return has_intersection;
}

Color
raytrace(const Scene &scene, Random &rnd, Ray ray, int depth = 0);

Color
diffuse_raytrace(const Scene &scene, Random &rnd, const std::shared_ptr<Primitive>& primitive,
		 glm::vec3 point, glm::vec3 normal, Ray ray, int depth)
{
	glm::vec3 w = {rnd.normal(), rnd.normal(), rnd.normal()};
	w = glm::normalize(w);
	auto w_normal_dot = glm::dot(w, normal);
	if (glm::dot(w, normal) < 0.f) {
		w *= -1.f;
		w_normal_dot *= -1.f;
	}
	Ray wRay = {w, point + w * EPS5};
	return primitive->emission + 2.f * w_normal_dot * raytrace(scene, rnd, wRay, depth + 1) * primitive->color;
}

Color
metallic_raytrace(const Scene &scene, Random &rnd, const std::shared_ptr<Primitive>& primitive,
		  glm::vec3 point, glm::vec3 normal, Ray ray, int depth)
{
	auto reflect_dir = ray.direction - 2.f * normal * glm::dot(normal, ray.direction);
	Ray reflect_ray = {reflect_dir, point + reflect_dir * EPS5};
	return primitive->emission + raytrace(scene, rnd, reflect_ray, depth + 1) * primitive->color;
}

Color
dielectric_raytrace(const Scene &scene, Random &rnd, const std::shared_ptr<Primitive>& primitive,
		    glm::vec3 point, glm::vec3 normal, Ray ray, bool inside, int depth)
{
	auto normal_ray_dot = glm::dot(normal, ray.direction);
	auto eta1 = 1.f, eta2 = primitive->ior;
	if (inside) {
		std::swap(eta1, eta2);
	}
	auto cosTheta1 = -normal_ray_dot;
	auto sinTheta1 = sqrtf(1.f - powf(cosTheta1, 2.f));
	auto sinTheta2 = eta1 / eta2 * sinTheta1;
	auto r0 = powf((eta1 - eta2) / (eta1 + eta2), 2.f);
	auto r = r0 + (1.f - r0) * powf(1.f + normal_ray_dot, 5.f);
	if (std::abs(sinTheta2) > 1.f || rnd.uniform() < r) {
		auto reflect_dir = ray.direction - 2.f * normal_ray_dot * normal;
		Ray reflect_ray = {reflect_dir, point + reflect_dir * EPS5};
		auto reflected = raytrace(scene, rnd, reflect_ray, depth + 1);
		return primitive->emission + reflected;
	}
	auto cosTheta2 = sqrtf(1.f - powf(sinTheta2, 2.f));
	auto refract_dir = eta1 / eta2 * (ray.direction) + (eta1 / eta2 * cosTheta1 - cosTheta2) * normal;
	Ray refract_ray = {refract_dir, point + refract_dir * EPS5};
	auto refracted = raytrace(scene, rnd, refract_ray, depth + 1);
	if (!inside) {
		refracted *= primitive->color;
	}
	return primitive->emission + refracted;
}

Color
raytrace(const Scene &scene, Random &rnd, Ray ray, int depth)
{
	if (depth >= scene.ray_depth)
		return black;

	glm::vec3 point, normal;
	bool inside;
	std::shared_ptr<Primitive> primitive;
	{
		Intersection intersection{};
		if (!intersect(scene, ray, intersection, primitive))
			return scene.bg_color;
		point = intersection.point;
		normal = intersection.normal;
		inside = intersection.inside;
	}

	assert(std::abs(glm::length(normal) - 1.f) < EPS5);
	assert(std::abs(glm::length(ray.direction) - 1.f) < EPS5);

	switch (primitive->material) {
		case (Material::DIFFUSE):
			return diffuse_raytrace(scene, rnd, primitive, point, normal, ray, depth);
		case (Material::METALLIC):
			return metallic_raytrace(scene, rnd, primitive, point, normal, ray, depth);
		case (Material::DIELECTRIC):
			return dielectric_raytrace(scene, rnd, primitive, point, normal, ray, inside, depth);
		default:
			unreachable();
	}
}

Image
render(const Scene &scene)
{
	const auto &camera = scene.camera;
	Image image(camera.height, camera.width);
	Random rnd;
	for (int i = 0; i < camera.height; i++) {
		for (int j = 0; j < camera.width; j++) {
			Color color = black;
			for (int k = 0; k < scene.samples; k++) {
				float x = (float)i + rnd.uniform();
				float y = (float)j + rnd.uniform();
				auto ray = camera.ray_throw(x, y);
				color += raytrace(scene, rnd, ray);
			}
			color /= (float)scene.samples;
			image.set_pixel(i, j, gamma_corrected(aces_tonemap(color)));
		}
	}
	return image;
}
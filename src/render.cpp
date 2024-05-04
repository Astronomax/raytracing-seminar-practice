#include <render.hpp>

#include <geometry_utils.hpp>
#include <Random.hpp>
#include <Ray.hpp>
#include <utils.hpp>

#include <iostream>

bool
intersect(const Scene &scene, Ray ray, Intersection &intersection)
{
	bool has_intersection = false;
	float min_distance = INF;
	for (auto &primitive : scene.planes) {
		auto intersection_opt = primitive.intersect(ray);
		if (!intersection_opt.has_value())
			continue;
		float distance = intersection_opt.value().distance;
		if (EPS5 < distance && distance < min_distance) {
			min_distance = distance;
			intersection = intersection_opt.value();
			has_intersection = true;
		}
	}
	auto intersection_opt = scene.bvh.intersect(ray, scene.bvh.root, min_distance);
	if (!intersection_opt.has_value())
		return has_intersection;
	float distance = intersection_opt.value().distance;
	if (EPS5 < distance && distance < min_distance) {
		min_distance = distance;
		intersection = intersection_opt.value();
		has_intersection = true;
	}
	return has_intersection;
}

Color
raytrace(const Scene &scene, Random &rnd, Ray ray, int depth = 0);

Color
diffuse_raytrace(const Scene &scene, Random &rnd,
		 const Primitive *primitive, glm::vec3 point,
		 glm::vec3 normal, Ray ray, int depth)
{
	auto w = scene.distribution.sample(rnd, point + EPS5 * normal, normal);
	assert(std::abs(glm::length(w) - 1.f) < EPS5);
	auto w_normal_dot = glm::dot(w, normal);
	if (w_normal_dot < 0.f)
		return primitive->emission;
	auto p = scene.distribution.pdf(point + EPS5 * normal, normal, w);
	Ray wRay = {w, point + w * EPS5};
	return primitive->emission + 1.f / (PI * p) * w_normal_dot * raytrace(scene, rnd, wRay, depth + 1) * primitive->color;
}

Color
metallic_raytrace(const Scene &scene, Random &rnd,
		  const Primitive *primitive, glm::vec3 point,
		  glm::vec3 normal, Ray ray, int depth)
{
	auto reflect_dir = ray.direction - 2.f * normal * glm::dot(normal, ray.direction);
	Ray reflect_ray = {reflect_dir, point + reflect_dir * EPS5};
	return primitive->emission + raytrace(scene, rnd, reflect_ray, depth + 1) * primitive->color;
}

Color
dielectric_raytrace(const Scene &scene, Random &rnd,
		    const Primitive *primitive, glm::vec3 point,
		    glm::vec3 normal, Ray ray, bool inside, int depth)
{
	auto normal_ray_dot = glm::dot(normal, ray.direction);
	auto eta1 = 1.f, eta2 = primitive->ior;
	if (inside)
		std::swap(eta1, eta2);
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
	if (!inside)
		refracted *= primitive->color;
	return primitive->emission + refracted;
}

Color
raytrace(const Scene &scene, Random &rnd, Ray ray, int depth)
{
	if (depth >= scene.ray_depth)
		return black;

	glm::vec3 point, normal;
	bool inside;
	const Primitive *primitive;
	{
		Intersection intersection{};
		if (!intersect(scene, ray, intersection))
			return scene.bg_color;
		point = intersection.point;
		normal = intersection.normal;
		inside = intersection.inside;
		primitive = intersection.obstacle;
	}

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
render(Scene &scene)
{
	const auto &camera = scene.camera;
	Image image(camera.height, camera.width);
	//Random rnd;
	#pragma omp parallel for schedule(dynamic, 8)
	for (int pixel = 0; pixel < camera.height * camera.width; pixel++) {
		Random rnd;
		int i = pixel / camera.width;
		int j = pixel % camera.width;
		Color color = black;
		for (int k = 0; k < scene.samples; k++) {
			float x = (float) i + rnd.uniform();
			float y = (float) j + rnd.uniform();
			auto ray = camera.ray_throw(x, y);
			color += raytrace(scene, rnd, ray);
		}
		color /= (float) scene.samples;
		image.set_pixel(i, j, gamma_corrected(aces_tonemap(color)));
	}
	return image;
}
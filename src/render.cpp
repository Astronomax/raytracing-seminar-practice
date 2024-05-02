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
	glm::vec3 w = scene.distrib->sample(point + EPS5 * normal, normal);
	//static int cnt = 0;
	//int tmp = cnt;
	//if(cnt++ < 10)
	//	std::cout << "w: " << w.x << " " << w.y << " " << w.z << "\n";
	assert(std::abs(glm::length(w) - 1.f) < EPS5);
	auto w_normal_dot = glm::dot(w, normal);
	if (w_normal_dot < 0.f) {
		return primitive->emission;
	}
	float p = scene.distrib->pdf(point + EPS5 * normal, normal, w);
	Ray wRay = {w, point + w * EPS5};
	//if(tmp < 10) std::cout << "pdf: " << p << "\n";
	//if(tmp < 10) std::cout << "1.f / (PI * p)...: " << 1.f / (PI * p) * w_normal_dot << std::endl;
	auto res = primitive->emission + 1.f / (PI * p) * w_normal_dot * raytrace(scene, rnd, wRay, depth + 1) * primitive->color;;
	//if(tmp < 10) std::cout << "res: " << res.x << " " << res.y << " " << res.z << std::endl;
	return res;
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
		//static int cnt = 0;
		//while(cnt++ < 10) {
		//	std::cout << "point: " << point.x << " " << point.y << " " << point.z << "\n";
		//	std::cout << "normal: " << normal.x << " " << normal.y << " " << normal.z << "\n";
		//}
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
render(Random &rnd, Scene &scene)
{
	const auto &camera = scene.camera;
	Image image(camera.height, camera.width);
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
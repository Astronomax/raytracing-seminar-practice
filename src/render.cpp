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
	//static int cnt = 0;
	//bool debug = (cnt++ == 15);
	//std::cout << ray.direction.x << " " << ray.direction.y << " " << ray.direction.z << std::endl;
	auto intersection_opt = scene.bvh.intersect(ray, scene.bvh.root, min_distance, false);//debug);
	//if (debug) exit(0);
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
raytrace(const Scene &scene, Random &rnd, Ray ray, int depth = 0, int start = 0);

Color
diffuse_raytrace(const Scene &scene, Random &rnd,
		 const Primitive *primitive, glm::vec3 point,
		 glm::vec3 normal, Ray ray, int depth, int start)
{
	//std::cout << "diffuse_raytrace" << std::endl;
	auto w = scene.distribution.sample(rnd, point + EPS5 * normal, normal);
	//std::cout << point.x << " " << point.y << " " << point.z << std::endl;
	//std::cout << w.x << " " << w.y << " " << w.z << std::endl;
	//exit(0);
	assert(std::abs(glm::length(w) - 1.f) < EPS5);
	auto w_normal_dot = glm::dot(w, normal);
	if (w_normal_dot < 0.f)
		return primitive->emission;
	auto p = scene.distribution.pdf(point + EPS5 * normal, normal, w);
	Ray wRay = {w, point + w * EPS5};
	return primitive->emission + 1.f / (PI * p) * w_normal_dot * raytrace(scene, rnd, wRay, depth + 1, start) * primitive->color;
}

Color
metallic_raytrace(const Scene &scene, Random &rnd,
		  const Primitive *primitive, glm::vec3 point,
		  glm::vec3 normal, Ray ray, int depth, int start)
{
	//std::cout << "metallic_raytrace" << std::endl;
	auto reflect_dir = ray.direction - 2.f * normal * glm::dot(normal, ray.direction);
	Ray reflect_ray = {reflect_dir, point + reflect_dir * EPS5};
	return primitive->emission + raytrace(scene, rnd, reflect_ray, depth + 1, start) * primitive->color;
}

Color
dielectric_raytrace(const Scene &scene, Random &rnd,
		    const Primitive *primitive, glm::vec3 point,
		    glm::vec3 normal, Ray ray, bool inside, int depth, int start)
{
	//std::cout << "dielectric_raytrace" << std::endl;
	auto normal_ray_dot = glm::dot(normal, ray.direction);
	auto eta1 = 1.f, eta2 = primitive->ior;
	if (inside)
		std::swap(eta1, eta2);
	auto cosTheta1 = -normal_ray_dot;
	auto sinTheta1 = sqrtf(1.f - powf(cosTheta1, 2.f));
	auto sinTheta2 = eta1 / eta2 * sinTheta1;
	auto r0 = powf((eta1 - eta2) / (eta1 + eta2), 2.f);
	auto r = r0 + (1.f - r0) * powf(1.f + normal_ray_dot, 5.f);
	auto u = rnd.uniform();
	//std::cout << sinTheta2 << " " << u << std::endl;
	if (std::abs(sinTheta2) > 1.f || u < r) {
		auto reflect_dir = ray.direction - 2.f * normal_ray_dot * normal;
		Ray reflect_ray = {reflect_dir, point + reflect_dir * EPS5};
		auto reflected = raytrace(scene, rnd, reflect_ray, depth + 1, start);
		return primitive->emission + reflected;
	}
	auto cosTheta2 = sqrtf(1.f - powf(sinTheta2, 2.f));
	auto refract_dir = eta1 / eta2 * (ray.direction) + (eta1 / eta2 * cosTheta1 - cosTheta2) * normal;
	Ray refract_ray = {refract_dir, point + refract_dir * EPS5};
	auto refracted = raytrace(scene, rnd, refract_ray, depth + 1, start);
	if (!inside)
		refracted *= primitive->color;
	return primitive->emission + refracted;
}

Color
raytrace(const Scene &scene, Random &rnd, Ray ray, int depth, int start)
{
	//if (((double)clock() - (double)start) / (double)CLOCKS_PER_SEC > 0.5f)
	//	return black;
	//std::cout << depth << std::endl;
	if (depth >= scene.ray_depth)
		return black;

	//glm::vec3 point, normal;
	//bool inside;
	//const Primitive *primitive;
	//{
	Intersection intersection{};
	if (!intersect(scene, ray, intersection))
		return scene.bg_color;
	const auto &[distance, point, normal,
		     inside, primitive] = intersection;
		//point = intersection.point;
		//normal = intersection.normal;
		//inside = intersection.inside;
		//primitive = intersection.obstacle;
	//}

	switch (primitive->material) {
		case (Material::DIFFUSE):
			return diffuse_raytrace(scene, rnd, primitive, point, normal, ray, depth, start);
		case (Material::METALLIC):
			return metallic_raytrace(scene, rnd, primitive, point, normal, ray, depth, start);
		case (Material::DIELECTRIC):
			return dielectric_raytrace(scene, rnd, primitive, point, normal, ray, inside, depth, start);
		default:
			unreachable();
	}
}

Image
render(Scene &scene)
{
	const auto &camera = scene.camera;
	Image image(camera.height, camera.width);

	int start = clock();
	#pragma omp parallel for schedule(dynamic,8)
	for (int pixel = 0; pixel < camera.height * camera.width; pixel++) {
		//std::cout << pixel << std::endl;
		Random rnd(pixel);
		int i = pixel / camera.width;
		int j = pixel % camera.width;
		Color color = black;
		for (int k = 0; k < scene.samples; k++) {
			float x = (float) j + rnd.uniform();
			float y = (float) i + rnd.uniform();
			auto ray = camera.ray_throw(x, y);
			//std::cout << x << " " << y << std::endl;
			//std::cout << ray.origin.x << " " << ray.origin.y << " " << ray.origin.z << std::endl;
			//std::cout << ray.direction.x << " " << ray.direction.y << " " << ray.direction.z << std::endl;
			//exit(0);
			color += raytrace(scene, rnd, ray, 0, start);
		}
		color /= (float) scene.samples;
		image.set_pixel(i, j, gamma_corrected(aces_tonemap(color)));
	}
	return image;
}
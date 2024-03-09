#include "Primitive.hpp"
#include "geometry_utils.hpp"

#include "glm/geometric.hpp"

#include <algorithm>
#include <cmath>

std::optional<Intersection> Primitive::intersect(Ray ray) const {
	auto in_local = to_local(ray, *this);
	auto intersection = intersect_ignore_transformation(in_local);
	if (!intersection.has_value())
		return std::nullopt;
	intersection->point = rotate(intersection->point, conjugate(rotation)) + position;
	intersection->normal = rotate(intersection->normal, conjugate(rotation));
	return intersection;
}

bool get_square_equation_roots(float a, float b, float c, float &t1, float &t2) {
	auto d = b * b - 4.f * a * c;
	if (d < 0.f)
		return false;
	t1 = (-b - sqrtf(d)) / (2.f * a);
	t2 = (-b + sqrtf(d)) / (2.f * a);
	return true;
}

bool min_greater_than_zero(float t1, float t2, float &t) {
	t = (t1 > 0.f) ? t1 : t2;
	return t > 0.f;
}

std::optional<Intersection> Ellipsoid::intersect_ignore_transformation(Ray ray) const {
	auto divided_ray = ray;
	divided_ray.origin /= radius;
	divided_ray.direction /= radius;
	float t1, t2, t;
	if (!get_square_equation_roots(glm::dot(divided_ray.direction, divided_ray.direction),
								   2.f * glm::dot(divided_ray.origin, divided_ray.direction),
								   glm::dot(divided_ray.origin, divided_ray.origin) - 1.f, t1, t2))
		return std::nullopt;

	if (t1 > t2)
		std::swap(t1, t2);
	if (!min_greater_than_zero(t1, t2, t))
		return std::nullopt;

	Intersection intersection{};
	intersection.distance = t;
	intersection.point = walk_along(ray, t);
	intersection.normal = glm::normalize(intersection.point / radius);
	intersection.inside = (t1 < 0.f || t2 < 0.f);
	if (glm::dot(intersection.normal, ray.direction) > 0.f)
		intersection.normal *= -1.f;
	return intersection;
}

std::optional<Intersection> Plane::intersect_ignore_transformation(Ray ray) const {
	auto t = -glm::dot(ray.origin, normal)
			/ glm::dot(ray.direction, normal);
	if (t < 0.f)
		return std::nullopt;

	Intersection intersection{};
	intersection.distance = t;
	intersection.point = walk_along(ray, t);
	intersection.normal = glm::normalize(normal);
	intersection.inside = false;
	if (glm::dot(intersection.normal, ray.direction) > 0.f)
		intersection.normal *= -1.f;
	return intersection;
}

std::optional<Intersection> Box::intersect_ignore_transformation(Ray ray) const {
	auto v1 = (diagonal - ray.origin) / ray.direction;
	auto v2 = (-diagonal - ray.origin) / ray.direction;
	if (v1.x > v2.x) std::swap(v1.x, v2.x);
	if (v1.y > v2.y) std::swap(v1.y, v2.y);
	if (v1.z > v2.z) std::swap(v1.z, v2.z);
	auto t1 = std::max(v1.x, std::max(v1.y, v1.z));
	auto t2 = std::min(v2.x, std::min(v2.y, v2.z));
	float t;

	if (t1 > t2)
		return std::nullopt;
	if (!min_greater_than_zero(t1, t2, t))
		return std::nullopt;

	Intersection intersection{};
	intersection.distance = t;
	intersection.point = walk_along(ray, t);
	auto normal = glm::abs(intersection.point / diagonal);
	auto max_component = std::max({normal.x, normal.y, normal.z});
	if (std::abs(normal.x - max_component) > 1e-7)
		normal.x = 0.f;
	if (std::abs(normal.y - max_component) > 1e-7)
		normal.y = 0.f;
	if (std::abs(normal.z - max_component) > 1e-7)
		normal.z = 0.f;
	if (glm::dot(normal, ray.direction) > 0.f)
		normal *= -1.f;
	intersection.normal = glm::normalize(normal);
	intersection.inside = (t1 < 0.f);
	return intersection;
}
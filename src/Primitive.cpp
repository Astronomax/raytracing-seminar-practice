#include <Primitive.hpp>

#include <geometry_utils.hpp>
#include <utils.hpp>

#include "glm/geometric.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

inline Ray
to_local(const Ray &ray, const Primitive &primitive)
{
	return {
		rotate(ray.direction, primitive.rotation),
		rotate(ray.origin - primitive.position, primitive.rotation),
	};
}

bool
get_square_equation_roots(float a, float b, float c, float &t1, float &t2)
{
	auto d = b * b - 4 * a * c;
	if (d < 0.f)
		return false;
	t1 = (-b - sqrtf(d)) / (2.f * a);
	t2 = (-b + sqrtf(d)) / (2.f * a);
	//assert(!std::isnan(t1));
	//assert(!std::isnan(t2));
	return true;
}

inline bool
min_geq_zero(float t1, float t2, float &t)
{
	//assert(!std::isnan(t1));
	//assert(!std::isnan(t2));
	t = (t1 >= 0.f) ? t1 : t2;
	return t >= 0.f;
}

std::optional<Intersection>
Primitive::intersect_ignore_transformation_ellipsoid(const Ray &ray) const
{
	auto divided_ray = ray;
	auto &radius = primitive_specific[0];
	divided_ray.origin /= radius;
	divided_ray.direction /= radius;
	float t1, t2, t;
	if (!get_square_equation_roots(glm::dot(divided_ray.direction, divided_ray.direction),
				       2.f * glm::dot(divided_ray.origin, divided_ray.direction),
				       glm::dot(divided_ray.origin, divided_ray.origin) - 1.f, t1, t2))
		return std::nullopt;

	if (t1 > t2)
		std::swap(t1, t2);
	if (!min_geq_zero(t1, t2, t))
		return std::nullopt;

	Intersection intersection{};
	intersection.distance = t;
	intersection.point = walk_along(ray, t);
	intersection.normal = glm::normalize(intersection.point / (radius * radius));
	intersection.inside = (t1 < 0.f);
	intersection.obstacle = this;
	if (intersection.inside)
		intersection.normal *= -1.f;
	return intersection;
}

std::optional<Intersection>
Primitive::intersect_ignore_transformation_plane(const Ray &ray) const
{
	auto &normal = primitive_specific[0];
	auto t = -glm::dot(ray.origin, normal)
		 / glm::dot(ray.direction, normal);
	if (t < 0.f || t > 1e4f)
		return std::nullopt;

	Intersection intersection{};
	intersection.distance = t;
	intersection.point = walk_along(ray, t);
	intersection.normal = glm::normalize(normal);
	intersection.inside = false;
	intersection.obstacle = this;
	if (glm::dot(intersection.normal, ray.direction) > 0.f)
		intersection.normal *= -1.f;
	return intersection;
}

std::optional<IntersectionSmall>
Primitive::intersect_ignore_transformation_box_small(const glm::vec3 &diagonal, const Ray &ray, bool debug)
{
	//auto &diagonal = primitive_specific[0];
	//if(debug) std::cout << "1: " << (diagonal).x << " " << (diagonal).y << " " << (diagonal).z << std::endl;
	//if(debug) std::cout << "1: " << (ray.origin).x << " " << (ray.origin).y << " " << (ray.origin).z << std::endl;
	//if(debug) std::cout << "1: " << (diagonal - ray.origin).x << " " << (diagonal - ray.origin).y << " " << (diagonal - ray.origin).z << std::endl;
	//if(debug) std::cout << "2: " <<  ray.direction.x << " " << ray.direction.y << " " << ray.direction.z << std::endl;
	auto v1 = (diagonal - ray.origin) / ray.direction;
	auto v2 = (-diagonal - ray.origin) / ray.direction;
	if (v1.x > v2.x) std::swap(v1.x, v2.x);
	if (v1.y > v2.y) std::swap(v1.y, v2.y);
	if (v1.z > v2.z) std::swap(v1.z, v2.z);
	auto t1 = std::max(v1.x, std::max(v1.y, v1.z));
	auto t2 = std::min(v2.x, std::min(v2.y, v2.z));
	float t;
	//if(debug)
	//std::cout << t1 << " " << t2 << std::endl;

	if (t1 > t2)
		return std::nullopt;
	if (!min_geq_zero(t1, t2, t))
		return std::nullopt;
	return {IntersectionSmall{t, (t1 < 0.f)}};
}

std::optional<Intersection>
Primitive::intersect_ignore_transformation_box(const Ray &ray, bool debug) const
{
	auto &diagonal = primitive_specific[0];

	auto small = intersect_ignore_transformation_box_small(diagonal, ray, debug);
	if (!small.has_value())
		return std::nullopt;
	Intersection intersection{};
	intersection.distance = small.value().distance;
	intersection.inside = small.value().inside;

	intersection.point = walk_along(ray, intersection.distance);
	auto normal = intersection.point / diagonal;
	auto max_component = std::max({std::abs(normal.x), std::abs(normal.y), std::abs(normal.z)});
	if (std::abs(std::abs(normal.x) - max_component) > EPS5)
		normal.x = 0.f;
	if (std::abs(std::abs(normal.y) - max_component) > EPS5)
		normal.y = 0.f;
	if (std::abs(std::abs(normal.z) - max_component) > EPS5)
		normal.z = 0.f;
	intersection.obstacle = this;
	if (intersection.inside)
		normal *= -1.f;
	intersection.normal = glm::normalize(normal);
	return intersection;
}

std::optional<Intersection>
Primitive::intersect_ignore_transformation_triangle(const Ray &ray) const
{
	const auto &a = primitive_specific[2];
	const auto &b = primitive_specific[0] - a;
	const auto &c = primitive_specific[1] - a;
	const auto normal = glm::cross(b, c);
	Primitive plane;
	plane.type = PrimitiveType::PLANE;
	plane.primitive_specific[0] = normal;
	auto intersection = plane.intersect_ignore_transformation_plane(
		{ray.direction, ray.origin - a});
	//intersection->point = walk_along(ray, intersection->distance);
	if (!intersection.has_value())
		return std::nullopt;
	auto p = intersection->point;
	if (glm::dot(glm::cross(b, p), normal) < 0)
		return std::nullopt;
	if (glm::dot(glm::cross(p, c), normal) < 0)
		return std::nullopt;
	if (glm::dot(glm::cross(c - b, p - b), normal) < 0)
		return std::nullopt;
	intersection->point += a;
	intersection->obstacle = this;
	return intersection;
}

std::optional<Intersection>
Primitive::intersect(const Ray &ray, bool debug) const
{
	auto in_local = to_local(ray, *this);

	std::optional<Intersection> intersection;
	switch (type) {
		case (PrimitiveType::ELLIPSOID):
			intersection = intersect_ignore_transformation_ellipsoid(in_local);
			break;
		case (PrimitiveType::PLANE):
			intersection = intersect_ignore_transformation_plane(in_local);
			break;
		case (PrimitiveType::BOX):
			intersection = intersect_ignore_transformation_box(in_local, debug);
			break;
		case (PrimitiveType::TRIANGLE):
			intersection = intersect_ignore_transformation_triangle(in_local);
			break;
		default:
			unreachable();
	}
	if (!intersection.has_value())
		return std::nullopt;
	intersection->point = walk_along(ray, intersection->distance);
	intersection->normal = rotate(intersection->normal, conjugate(rotation));
	return intersection;
}

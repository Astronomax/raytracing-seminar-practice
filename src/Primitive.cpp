#include "Primitive.hpp"

#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"

glm::vec3 rotate(glm::vec3 v, glm::quat q) {
	glm::quat res = (glm::inverse(q) * glm::quat(0, v) * q);
	return {res.x, res.y, res.z};
}

std::optional<float> Ellipsoid::intersect(const Ray &ray) const {
	auto origin = rotate(ray.origin - position, rotation) / radius;
	auto direction = rotate(ray.direction, rotation) / radius;
	float c = glm::dot(origin, origin) - 1.f;
	float b = 2.f * glm::dot(origin, direction);
	float a = glm::dot(direction, direction);
	float d = b * b - 4.f * a * c;
	if (d < 0.f)
		return std::nullopt;
	float t1 = (-b - sqrtf(d)) / (2.f * a);
	float t2 = (-b + sqrtf(d)) / (2.f * a);
	if (t1 > t2)
		std::swap(t1, t2);
	if (t1 > 0.f)
		return t1;
	if (t2 > 0.f)
		return t2;
	return std::nullopt;
}

std::optional<float> Plane::intersect(const Ray &ray) const {
	auto origin = rotate(glm::vec3(ray.origin - position), rotation);
	auto direction = rotate(ray.direction, rotation);
	float t = -glm::dot(origin, normal)
			/ glm::dot(direction, normal);
	if (t < 0.f)
		return std::nullopt;
	return t;
}

std::optional<float> Box::intersect(const Ray &ray) const {
	auto origin = rotate(glm::vec3(ray.origin - position), rotation);
	auto direction = rotate(ray.direction, rotation);
	auto v1 = (diagonal - origin) / direction;
	auto v2 = (-diagonal - origin) / direction;
	if (v1.x > v2.x) std::swap(v1.x, v2.x);
	if (v1.y > v2.y) std::swap(v1.y, v2.y);
	if (v1.z > v2.z) std::swap(v1.z, v2.z);
	auto t1 = std::max(v1.x, std::max(v1.y, v1.z));
	auto t2 = std::min(v2.x, std::min(v2.y, v2.z));
	if (t1 > t2)
		return std::nullopt;
	if (t1 > 0.f)
		return t1;
	if (t2 > 0.f)
		return t2;
	return std::nullopt;
}
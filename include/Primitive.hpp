#ifndef RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

#include "Color.hpp"
#include "Ray.hpp"
#include "Transformable.hpp"

#include <optional>


struct Primitive : public Transformable {
	virtual std::optional<float> intersect(const Ray &ray) const = 0;
	Color color = black;
};

struct Ellipsoid : public Primitive {
	std::optional<float> intersect(const Ray &ray) const override;
	glm::vec3 radius;
};

struct Plane : public Primitive {
	std::optional<float> intersect(const Ray &ray) const override;
	glm::vec3 normal;
};

struct Box : public Primitive {
	std::optional<float> intersect(const Ray &ray) const override;
	glm::vec3 diagonal;
};

#endif //RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

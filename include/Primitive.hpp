#ifndef RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

#include "Color.hpp"
#include "Ray.hpp"
#include "Transformable.hpp"

#include <optional>

enum class Material {
	METALLIC,
	DIELECTRIC,
	DIFFUSE,
	MATERIALS_NUMBER
};

struct Intersection {
	float distance;
	glm::vec3 point;
	glm::vec3 normal;
	bool inside;
};

struct Primitive : public Transformable {
	std::optional<Intersection> intersect(Ray ray) const;
	virtual std::optional<Intersection> intersect_ignore_transformation(Ray ray) const = 0;
	Color color = black;
	Material material = Material::DIFFUSE;
	float ior;
};

struct Ellipsoid : public Primitive {
	std::optional<Intersection> intersect_ignore_transformation(Ray ray) const override;
	glm::vec3 radius;
};

struct Plane : public Primitive {
	std::optional<Intersection> intersect_ignore_transformation(Ray ray) const override;
	glm::vec3 normal;
};

struct Box : public Primitive {
	std::optional<Intersection> intersect_ignore_transformation(Ray ray) const override;
	glm::vec3 diagonal;
};

#endif //RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

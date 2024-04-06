#ifndef RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

#include "Color.hpp"
#include "Ray.hpp"

#include <glm/gtc/quaternion.hpp>

#include <optional>

enum class PrimitiveType {
    ELLIPSOID,
    PLANE,
    BOX,
    PRIMITIVES_NUMBER
};

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

struct Primitive {
	std::optional<Intersection> intersect(Ray ray) const;
	std::optional<Intersection> intersect_ignore_transformation_ellipsoid(Ray ray) const;
	std::optional<Intersection> intersect_ignore_transformation_plane(Ray ray) const;
	std::optional<Intersection> intersect_ignore_transformation_box(Ray ray) const;
	Color color = black;
	Material material = Material::DIFFUSE;
	float ior;
	Color emission;
    	PrimitiveType type;
	glm::vec3 primitive_specific;
	glm::vec3 position = {0, 0, 0};
	glm::quat rotation = {1, 0, 0, 0};
};

#endif //RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

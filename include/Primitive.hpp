#ifndef RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

#include "Color.hpp"
#include "Ray.hpp"

#include <glm/gtc/quaternion.hpp>

#include <optional>
#include <memory>

enum class PrimitiveType {
    ELLIPSOID,
    PLANE,
    BOX,
    TRIANGLE,
    PRIMITIVES_NUMBER
};

enum class Material {
	METALLIC,
	DIELECTRIC,
	DIFFUSE,
	MATERIALS_NUMBER
};

struct Primitive;
//typedef std::shared_ptr<const Primitive> PrimitivePtr;

struct IntersectionSmall {
    float distance;
    bool inside;
};

struct Intersection {
	float distance;
	glm::vec3 point;
	glm::vec3 normal;
	bool inside;
	const Primitive *obstacle;
};

struct Primitive {
	std::optional<Intersection> intersect(const Ray &ray, bool debug=false) const;

	static std::optional<IntersectionSmall> intersect_ignore_transformation_box_small(const glm::vec3 &diagonal, const Ray &ray, bool debug=false);
private:
	std::optional<Intersection> intersect_ignore_transformation_ellipsoid(const Ray &ray) const;
	std::optional<Intersection> intersect_ignore_transformation_plane(const Ray &ray) const;
	std::optional<Intersection> intersect_ignore_transformation_box(const Ray &ray, bool debug=false) const;
	std::optional<Intersection> intersect_ignore_transformation_triangle(const Ray &ray) const;
public:
	Color color = black;
	Material material = Material::DIFFUSE;
	float ior;
	Color emission;
    	PrimitiveType type;
	glm::vec3 primitive_specific[3];
	glm::vec3 position = {0, 0, 0};
	glm::quat rotation = {1, 0, 0, 0};
};

#endif //RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

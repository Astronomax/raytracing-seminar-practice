#ifndef RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP
#define RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

#include "Color.hpp"
#include "Material.hpp"
#include "Ray.hpp"
#include "Gltf.hpp"

#include <glm/gtc/quaternion.hpp>

#include <optional>
#include <memory>

enum class FigureType {
    ELLIPSOID,
    PLANE,
    BOX,
    TRIANGLE,
    PRIMITIVES_NUMBER
};

struct Primitive;

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
    
    	GltfMaterial material;
    	FigureType type;
	glm::vec3 primitive_specific[3];
	glm::vec3 position = {0, 0, 0};
	glm::quat rotation = {1, 0, 0, 0};
};

#endif //RAYTRACING_SEMINAR_PRACTICE_PRIMITIVE_HPP

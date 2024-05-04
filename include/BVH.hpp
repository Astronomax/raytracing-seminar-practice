#ifndef RAYTRACING_BVH_HPP
#define RAYTRACING_BVH_HPP

#include <Primitive.hpp>
#include <utils.hpp>

#include "glm/vec3.hpp"
#include "glm/glm.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

struct AABB {
    	void extend(glm::vec3 p);

	void extend(AABB aabb);

	std::optional<Intersection> intersect(Ray ray) const;

	glm::vec3 aabb_min = glm::vec3(INF, INF, INF);
	glm::vec3 aabb_max = glm::vec3(-INF, -INF, -INF);
};

AABB build_aabb(const Primitive* primitive);

struct Node {
	AABB aabb;
	int left_child = -1;
	int right_child = -1;
	int first_primitive_id = 0;
	int primitive_count = 0;
};

struct BVH {
	BVH() = default;
	BVH(const std::vector<const Primitive*> &primitives);
    	int build_node(std::unordered_map<const Primitive*, AABB> &aabbs, int first, int count);
	std::optional<Intersection> intersect(Ray ray, int current_id = -1, float min_distance = INF, bool debug = false) const;
	std::vector<Node> nodes;
	int root;

	std::vector<const Primitive*> primitives;
};

#endif //RAYTRACING_BVH_HPP

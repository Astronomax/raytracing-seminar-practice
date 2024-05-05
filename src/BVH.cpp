#include <BVH.hpp>
#include <geometry_utils.hpp>
#include <algorithm>
#include <iostream>

void AABB::extend(glm::vec3 p) {
	aabb_min = glm::min(aabb_min, p);
	aabb_max = glm::max(aabb_max, p);
}

void AABB::extend(AABB aabb) {
	aabb_min = glm::min(aabb_min, aabb.aabb_min);
	aabb_max = glm::max(aabb_max, aabb.aabb_max);
}

std::optional<IntersectionSmall> AABB::intersect(Ray ray) const {
	return Primitive::intersect_ignore_transformation_box_small(0.5f * (aabb_max - aabb_min),
		{ray.direction, ray.origin - 0.5f * (aabb_max + aabb_min)});
	//intersection->point = walk_along(ray, intersection->distance);
	//return intersection;
}

enum class Axis {
    X,
    Y,
    Z,
    AXIS_COUNT
};

AABB build_aabb(const Primitive* primitive) {
	AABB aabb_ignore_transformation;
	switch(primitive->type) {
		case (PrimitiveType::PLANE):
			unreachable();
		case (PrimitiveType::BOX):
		case (PrimitiveType::ELLIPSOID):
			aabb_ignore_transformation.extend(-primitive->primitive_specific[0]);
			aabb_ignore_transformation.extend(primitive->primitive_specific[0]);
			break;
		case (PrimitiveType::TRIANGLE): {
			auto aabb_min = glm::vec3(INF, INF, INF);
			for (int axis = 0; axis < (int) Axis::AXIS_COUNT; axis++)
				for (auto v: primitive->primitive_specific)
					aabb_min[axis] = std::min(aabb_min[axis], v[axis]);
			aabb_ignore_transformation.extend(aabb_min);
			auto aabb_max = glm::vec3(-INF, -INF, -INF);
			for (int axis = 0; axis < (int) Axis::AXIS_COUNT; axis++)
				for (auto v: primitive->primitive_specific)
					aabb_max[axis] = std::max(aabb_max[axis], v[axis]);
			aabb_ignore_transformation.extend(aabb_max);
			break;
		}
		default:
			unreachable();
	}
	AABB aabb;
	for (unsigned char mask = 0; mask < 8; mask++) {
		glm::vec3 p;
		for (int axis = 0; axis < (int)Axis::AXIS_COUNT; axis++) {
			p[axis] = (mask & (1 << axis)) ?
				  aabb_ignore_transformation.aabb_min[axis] :
				  aabb_ignore_transformation.aabb_max[axis];
		}
		p = rotate(p, conjugate(primitive->rotation)) + primitive->position;
		aabb.extend(p);
	}
	return aabb;
}

float aabb_surface_area(const AABB &aabb) {
	auto s = aabb.aabb_max - aabb.aabb_min;
	assert(s.x >= 0.f && s.y >= 0.f && s.z >= 0.f);
	return 2.f * (s.x * s.y + s.x * s.z + s.y * s.z);
}

typedef std::tuple<float, Axis, int> Split;

Split
seek_for_best_split(std::vector<const Primitive*> &primitives,
		 std::unordered_map<const Primitive*, AABB> &aabbs,
		 int first, int count)
{
	Split result = {INF, (Axis)0, -1};
	for (int i = 0; i < (int)Axis::AXIS_COUNT; i++) {
		std::sort(primitives.begin() + first,
			  primitives.begin() + first + count,
			  [i](const Primitive* &a, const Primitive* &b)
		{ return a->position[i] < b->position[i]; });
		std::vector<float> scores(count + 1);
		AABB aabb;
		for(int j = 0; j < count; j++) {
			aabb.extend(aabbs[primitives[first + j]]);
			scores[j + 1] += aabb_surface_area(aabb) * (float)(j + 1);
		}
		aabb = AABB();
		for(int j = count - 1; j >= 0; j--) {
			aabb.extend(aabbs[primitives[first + j]]);
			scores[j] += aabb_surface_area(aabb) * (float)(count - j);
		}
		for(int j = 1; j < count; j++)
			result = std::min(result, Split{scores[j], (Axis)i, j});
	}
	return result;
}

int
BVH::build_node(std::unordered_map<const Primitive*, AABB> &aabbs,
		int first, int count)
{
	Node current;
	current.first_primitive_id = first;
	current.primitive_count = count;
	for (int i = first; i < first + count; i++)
		current.aabb.extend(aabbs[primitives[i]]);
	int id = (int)nodes.size();
	nodes.push_back(current);
	if (count <= 1)
		return id;
	auto split = seek_for_best_split(primitives, aabbs, first, count);
	float current_score = aabb_surface_area(current.aabb) * (float)count;
	if (std::get<0>(split) >= current_score)
		return id;
	auto split_axis = (int)std::get<1>(split);
	std::sort(primitives.begin() + first,
		  primitives.begin() + first + count,
		  [i = split_axis](const Primitive* &a, const Primitive* &b)
		  { return a->position[i] < b->position[i]; });
	int left_count = std::get<2>(split);
	nodes[id].left_child = build_node(aabbs, first, left_count);
	nodes[id].right_child = build_node(aabbs, first + left_count, count - left_count);
	return id;
}

BVH::BVH(const std::vector<const Primitive*> &primitives_) : primitives(primitives_)
{
	std::unordered_map<const Primitive*, AABB> aabbs;
	for(auto &primitive : primitives)
		aabbs[primitive] = build_aabb(primitive);
	root = build_node(aabbs, 0, (int)primitives.size());
}

std::optional<Intersection>
BVH::intersect(Ray ray, int current_id, float min_distance, bool debug) const
{
	//if (debug) {
	//	std::cout << "come into " << current_id << std::endl;
	//}
	const auto &current = nodes[(current_id == -1) ? root : current_id];
	auto aabb_intersection = current.aabb.intersect(ray);
	if (!aabb_intersection.has_value()) {
		//if (debug) {
		//	std::cout << "exit from " << current_id << std::endl;
		//}
		return std::nullopt;
	}
	auto [t, inside] = aabb_intersection.value();
	if (min_distance < t && !inside) {
		//if (debug) {
		//	std::cout << "exit from " << current_id << std::endl;
		//}
		return std::nullopt;
	}
	std::optional<Intersection> result = std::nullopt;
	if (current.left_child != -1 && current.right_child != -1) {
		//if (debug) {
		//	std::cout << "iterate over children of " << current_id << std::endl;
		//}
		int children[2] = {current.left_child, current.right_child};
		for (auto &child_id : children) {
			//if (debug) {
			//	std::cout << "go to child " << child_id << std::endl;
			//}
			auto intersection = intersect(ray, child_id, min_distance, debug);
			if (intersection.has_value() && intersection->distance < min_distance) {
				min_distance = intersection->distance;
				result = intersection;
			}
			//if (debug) {
			//	std::cout << "min_distance = " << min_distance << std::endl;
			//}
		}
	} else {
		//if (debug) {
		//	std::cout << "iterate over primitives of " << current_id << std::endl;
		//}
		for (int i = 0; i < current.primitive_count; i++) {
			auto &primitive = primitives[current.first_primitive_id + i];
			auto intersection = primitive->intersect(ray);
			if (intersection.has_value() && intersection->distance < min_distance) {
				min_distance = intersection->distance;
				result = intersection;
			}
			//if (debug) {
			//	std::cout << "min_distance = " << min_distance << std::endl;
			//}
		}
	}
	//if (debug) {
	//	std::cout << "exit from " << current_id << std::endl;
	//}
	//if (result.has_value() && std::abs(min_distance - result.value().distance) > EPS7)
	//	throw std::logic_error("fail!");
	return result;
}
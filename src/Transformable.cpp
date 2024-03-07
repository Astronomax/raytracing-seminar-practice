#include "Transformable.hpp"
#include "geometry_utils.hpp"

Ray to_local(Ray ray, const Transformable &transformable) {
	return {
		rotate(ray.direction, transformable.rotation),
		rotate(ray.origin - transformable.position, transformable.rotation),
	};
}
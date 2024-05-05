#include <Scene.hpp>
#include <memory>

Distribution
build_distribution(const Scene &scene)
{
	std::vector<Distribution> primitive_distributions;
	for (const auto &primitive : scene.primitives) {
		if (primitive.emission.x > 0.f || primitive.emission.y > 0.f || primitive.emission.z > 0.f) {
			switch (primitive.type) {
				case (PrimitiveType::BOX): {
					Distribution box(DistributionType::BOX);
					box.init_box(&primitive);
					primitive_distributions.push_back(std::move(box));
					break;
				}
				case (PrimitiveType::ELLIPSOID): {
					Distribution ellipsoid(DistributionType::ELLIPSOID);
					ellipsoid.init_ellipsoid(&primitive);
					primitive_distributions.push_back(std::move(ellipsoid));
					break;
				}
				case (PrimitiveType::TRIANGLE): {
					Distribution triangle(DistributionType::TRIANGLE);
					triangle.init_triangle(&primitive);
					primitive_distributions.push_back(std::move(triangle));
					break;
				}
				default:
					unreachable();
			}
		}
	}
	std::vector<Distribution> distributions;
	{
		Distribution cosine(DistributionType::COSINE);
		distributions.push_back(std::move(cosine));
	}
	if (!primitive_distributions.empty()) {
		Distribution mixed(DistributionType::MIXED_ON_PRIMITIVES);
		mixed.init_mixed_on_primitives(primitive_distributions);
		distributions.push_back(std::move(mixed));
	}
	{
		Distribution mixed(DistributionType::MIXED);
		mixed.init_mixed(std::move(distributions));
		return mixed;
	}
}

void
Scene::init() {
	distribution = build_distribution(*this);
	std::vector<const Primitive*> primitives_;
	primitives_.reserve(primitives.size());
	for(auto &primitive : primitives)
		primitives_.push_back(&primitive);
	bvh = BVH(primitives_);
	//for(auto &node : bvh_.nodes)
	//	std::cout << node.aabb.aabb_min.x << " "
	//		<< node.aabb.aabb_min.y << " "
	//		<< node.aabb.aabb_min.z << " "
	//		<< node.aabb.aabb_max.x << " "
	//		<< node.aabb.aabb_max.y << " "
	//		<< node.aabb.aabb_max.z << "\n";
}
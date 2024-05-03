#include <Scene.hpp>
#include <memory>

std::unique_ptr<Distribution>
build_distribution(const std::shared_ptr<Random>& rnd, const Scene &scene)
{
	std::vector<std::unique_ptr<Distribution>> primitive_distributions;
	for (const auto &primitive : scene.primitives) {
		if (primitive->emission.x > 0.f || primitive->emission.y > 0.f || primitive->emission.z > 0.f) {
			switch (primitive->type) {
				case (PrimitiveType::BOX): {
					auto box = std::make_unique<Distribution>(rnd, DistributionType::BOX);
					box->init_box(primitive);
					primitive_distributions.push_back(std::move(box));
					break;
				}
				case (PrimitiveType::ELLIPSOID): {
					auto ellipsoid = std::make_unique<Distribution>(rnd, DistributionType::ELLIPSOID);
					ellipsoid->init_ellipsoid(primitive);
					primitive_distributions.push_back(std::move(ellipsoid));
					break;
				}
				default:
					unreachable();
			}
		}
	}
	std::vector<std::unique_ptr<Distribution>> distributions;
	{
		auto cosine = std::make_unique<Distribution>(rnd, DistributionType::COSINE);
		distributions.push_back(std::move(cosine));
	}
	if (!primitive_distributions.empty()) {
		auto mixed = std::make_unique<Distribution>(rnd, DistributionType::MIXED_ON_PRIMITIVES);
		mixed->init_mixed_on_primitives(primitive_distributions);
		distributions.push_back(std::move(mixed));
	}
	{
		auto mixed = std::make_unique<Distribution>(rnd, DistributionType::MIXED);
		mixed->init_mixed(std::move(distributions));
		return mixed;
	}
}

void
Scene::init(const std::shared_ptr<Random>& rnd) {
	distribution = build_distribution(rnd, *this);
	bvh = BVH(primitives);
}
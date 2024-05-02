#include <Scene.hpp>
#include <memory>

void Scene::init(Random &rnd) {
	std::vector<std::unique_ptr<Distrib>> primitive_distribs;
	for (const auto &primitive : primitives) {
		if (primitive->emission.x > 0 || primitive->emission.y > 0 || primitive->emission.z > 0) {
			switch (primitive->type) {
				case (PrimitiveType::BOX): {
					auto box = std::make_unique<Distrib>(rnd, DistribType::BOX);
					box->init_box(*primitive);
					primitive_distribs.push_back(std::move(box));
					break;
				}
				case (PrimitiveType::ELLIPSOID): {
					auto ellipsoid = std::make_unique<Distrib>(rnd, DistribType::ELLIPSOID);
					ellipsoid->init_ellipsoid(*primitive);
					primitive_distribs.push_back(std::move(ellipsoid));
					break;
				}
				default:
					unreachable();
			}
		}
	}
	std::vector<std::unique_ptr<Distrib>> distribs;
	{
		auto cosine = std::make_unique<Distrib>(rnd, DistribType::COSINE);
		distribs.push_back(std::move(cosine));
	}
	if (!primitive_distribs.empty()) {
		auto mixed = std::make_unique<Distrib>(rnd, DistribType::MIXED);
		mixed->init_mixed(std::move(primitive_distribs));
		distribs.push_back(std::move(mixed));
	}
	{
		auto mixed = std::make_unique<Distrib>(rnd, DistribType::MIXED);
		mixed->init_mixed(std::move(distribs));
		distrib = std::move(mixed);
	}
}
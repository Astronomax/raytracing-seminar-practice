#include <Random.hpp>

Random::Random() : rnd(240), uniform_(0.f, 1.f), normal_(0.f, 1.f) {}

float Random::uniform(float l, float r) {
	assert(l <= r);
	return l + uniform_(rnd) * (r - l);
}

float Random::normal() {
	return normal_(rnd);
}

Distrib::Distrib(Random &rnd, DistribType type)
	: rnd(rnd), type(type) {}

void Distrib::init_box(Primitive box) {
	primitive = box;
}

void Distrib::init_ellipsoid(Primitive ellipsoid) {
	primitive = ellipsoid;
}

void Distrib::init_mixed(std::vector<std::unique_ptr<Distrib>> &&distribs_) {
	distribs = std::move(distribs_);
}

glm::vec3 Distrib::sample_cosine(glm::vec3 n_x) const {
	//static int cnt = 0;
	//int tmp = cnt;
	//if(cnt++ < 10)
	//	std::cout << "COSINE\n";
	float x_ = rnd.normal(), y_ = rnd.normal(), z_ = rnd.normal();
	glm::vec3 w = {x_, y_, z_};
	//if(tmp < 10) std::cout << w.x << " " << w.y << " " << w.z << "\n";
	w = glm::normalize(w);
	//if(tmp < 10) std::cout << w.x << " " << w.y << " " << w.z << "\n";
	w = w + n_x;
	//if(tmp < 10) std::cout << w.x << " " << w.y << " " << w.z << "\n";
	auto len = glm::length(w);
	if (std::isnan(len) || len <= EPS5 || glm::dot(w, n_x) <= EPS5) {
		return n_x;
	}
	return glm::normalize(w);
}

glm::vec3 Distrib::sample_box(glm::vec3 x) const {
	//static int cnt = 0;
	//if(cnt++ < 10)
	//	std::cout << "BOX\n";
	auto &s = primitive.primitive_specific;
	glm::vec3 weight = {s.y * s.z, s.x * s.z, s.x * s.y};
	while (true) {
		float u = rnd.uniform(0.f, weight.x + weight.y + weight.z);
		float sign = (rnd.uniform() > 0.5f) ? 1.f : -1.f;
		glm::vec3 y;
		if (u < weight.x) {
			y = glm::vec3(sign * s.x, rnd.uniform(-s.y, s.y), rnd.uniform(-s.z, s.z));
		} else if (u < weight.x + weight.y) {
			y = glm::vec3(rnd.uniform(-s.x, s.x), sign * s.y, rnd.uniform(-s.z, s.z));
		} else {
			y = glm::vec3(rnd.uniform(-s.x, s.x), rnd.uniform(-s.y, s.y), sign * s.z);
		}
		y = rotate(y, conjugate(primitive.rotation)) + primitive.position;
		auto w = glm::normalize(y - x);
		if (primitive.intersect(Ray{w, x}).has_value()) {
			return w;
		}
	}
}

glm::vec3 Distrib::sample_ellipsoid(glm::vec3 x) const {
	//static int cnt = 0;
	//int tmp = cnt;
	//if(cnt++ < 10)
	//	std::cout << "ELLIPSOID\n";
	auto r = primitive.primitive_specific;
	//if(tmp < 10) std::cout << "=========\n";
	while (true) {
		float x_ = rnd.normal(), y_ = rnd.normal(), z_ = rnd.normal();
		auto y = glm::vec3(x_, y_, z_);

		//if(tmp < 10) {
		//	std::cout << y.x << " " << y.y << " " << y.z << " ";
		//	std::cout << rnd.normal() << "\n";
		//}
		y = glm::normalize(y);
		y *= r;
		y = rotate(y, conjugate(primitive.rotation)) + primitive.position;
		auto w = glm::normalize(y - x);
		//if(tmp < 10) std::cout << w.x << " " << w.y << " " << w.z << "\n";
		if (primitive.intersect(Ray{w, x}).has_value()) {
			//if(tmp < 10) std::cout << "=========\n";
			return w;
		}
	}
}

glm::vec3 Distrib::sample_mixed(glm::vec3 x, glm::vec3 n_x) const {
	//static int cnt = 0;
	float u = rnd.uniform(0, 1.f - EPS5);
	auto i = (int)(u * (float)distribs.size());
	//if (cnt++ < 100)
	//	std::cout << "MIXED " << u << " " << rnd.normal() << " " << i << "\n";
	assert(0 <= i && i < (int)distribs.size());
	i = std::max(0, std::min((int)distribs.size() - 1, i));
	return distribs[i]->sample(x, n_x);
}

glm::vec3 Distrib::sample(glm::vec3 x, glm::vec3 n_x) const {
	switch (type) {
		case (DistribType::COSINE):
			return sample_cosine(n_x);
		case (DistribType::BOX):
			return sample_box(x);
		case (DistribType::ELLIPSOID):
			return sample_ellipsoid(x);
		case (DistribType::MIXED):
			return sample_mixed(x, n_x);
		default:
			unreachable();
	}
}

float Distrib::pdf1_box(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const {
	auto &s = primitive.primitive_specific;
	auto area = 8 * (s.y * s.z + s.x * s.z + s.x * s.y);
	auto w = y - x;
	auto t = glm::length(x - y);
	w = glm::normalize(w);
	if (std::isnan(glm::length(w))) {
		return 0.f;
	}
	return powf(t, 2.f) / (area * std::abs(glm::dot(w, n_y)) + EPS7);
}

float Distrib::pdf1_ellipsoid(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const {
	auto r = primitive.primitive_specific;
	auto n = rotate(y - primitive.position, primitive.rotation) / r;
	auto p = 1.f / (4.f * PI * glm::length(glm::vec3(n.x * r.y * r.z, r.x * n.y * r.z, r.x * r.y * n.z)));
	auto w = y - x;
	auto t = glm::length(w);
	w = glm::normalize(w);

	if (std::isnan(glm::length(w))) {
		return 0.f;
	}

	assert(!std::isnan(p));
	assert(!std::isnan(t));
	auto res = p * powf(t, 2.f);
	assert(!std::isnan(res));

	assert(!std::isnan(glm::dot(w, n_y)));
	res /= (std::abs(glm::dot(w, n_y)) + EPS5);
	assert(!std::isnan(res));
	return res;
}

float Distrib::pdf_cosine(glm::vec3 n_x, glm::vec3 w) const {
	return std::max(0.f, glm::dot(w, n_x) / PI);
}

float Distrib::pdf1_primitive(glm::vec3 x, glm::vec3 w, glm::vec3 n_y) const {
	switch (type) {
		case (DistribType::BOX):
			return pdf1_box(x, w, n_y);
		case (DistribType::ELLIPSOID):
			return pdf1_ellipsoid(x, w, n_y);
		default:
			unreachable();
	}
}

float Distrib::pdf_mixed(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const {
	float p = 0;
	for (const auto &distrib : distribs) {
		auto p_ = distrib->pdf(x, n_x, w);
		assert(!std::isnan(p_));
		p += p_;
	}
	assert(!std::isnan(p));
	assert(distribs.size() != 0);
	return p / (float)distribs.size();
}

float Distrib::pdf(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const {
	switch (type) {
		case (DistribType::COSINE): {
			auto p = pdf_cosine(n_x, w);
			assert(!std::isnan(p));
			return p;
		}
		case (DistribType::BOX):
		case (DistribType::ELLIPSOID): {
			//static int cnt = 0;
			//cnt++;
			//if (cnt <= 25) std::cout << "ELLIPSOID pdf" << std::endl;
			auto origin = x;
			float p = 0.f;
			for (int i = 0; i < 2; i++) {
				auto intersection = primitive.intersect({w, origin});
				if (!intersection.has_value()) {
					//if (cnt <= 25) std::cout << "no intersection" << std::endl;
					//assert(!std::isnan(p));
					return 0.;
					//return p;
				}
				float t = intersection.value().distance;
				if (std::isnan(t)) {
					//if (cnt <= 25) std::cout << "no intersection" << std::endl;
					assert(!std::isnan(p));
					return p;
				}
				auto y = intersection.value().point;
				auto n_y = intersection.value().normal;
				//if (cnt <= 25) std::cout << "pdf1: " << pdf1_primitive(x, y, intersection->normal) << " ";
				p += pdf1_primitive(x, y, n_y);
				origin = intersection->point + w * EPS4;
			}
			//if (cnt <= 25)
			//	std::cout << std::endl;
			assert(!std::isnan(p));
			return p;
		}
		case (DistribType::MIXED): {
			auto p = pdf_mixed(x, n_x, w);
			assert(!std::isnan(p));
			return p;
		}
		default:
			unreachable();
	}
}
#include <Random.hpp>
#include <utility>

Random::Random()
	: rnd(std::chrono::steady_clock::now().time_since_epoch().count()),
	uniform_(0.f, 1.f), normal_(0.f, 1.f) {}

float
Random::uniform(float l, float r)
{
	assert(l <= r);
	return l + uniform_(rnd) * (r - l);
}

float
Random::normal(float mu, float sigma)
{
	return mu + normal_(rnd) * sigma;
}

Distribution::Distribution(std::shared_ptr<Random> rnd, DistributionType type)
	: rnd_(std::move(rnd)), type_(type) {}

void
Distribution::init_box(PrimitivePtr box)
{
	primitive_ = std::move(box);
}

void
Distribution::init_ellipsoid(PrimitivePtr ellipsoid)
{
	primitive_ = std::move(ellipsoid);
}

void
Distribution::init_mixed(std::vector<std::unique_ptr<Distribution>> &&distributions)
{
	distributions_ = std::move(distributions);
}

void
Distribution::init_mixed_on_primitives(const std::vector<std::unique_ptr<Distribution>> &distributions)
{
	std::vector<PrimitivePtr> primitives;
	for(auto &distribution : distributions)
		primitives.push_back(distribution->primitive_);
	bvh = BVH(primitives);
	distributions_.reserve(distributions.size());
	for (auto &primitive : bvh.primitives) {
		switch (primitive->type) {
			case (PrimitiveType::BOX): {
				auto distribution = std::make_unique<Distribution>(rnd_, DistributionType::BOX);
				distribution->init_box(primitive);
				distributions_.push_back(std::move(distribution));
				break;
			}
			case (PrimitiveType::ELLIPSOID): {
				auto distribution = std::make_unique<Distribution>(rnd_, DistributionType::ELLIPSOID);
				distribution->init_ellipsoid(primitive);
				distributions_.push_back(std::move(distribution));
				break;
			}
			default:
				unreachable();
		}
	}
}

glm::vec3
Distribution::sample_cosine(glm::vec3 n_x) const
{
	auto w = glm::normalize(glm::vec3(rnd_->normal(), rnd_->normal(), rnd_->normal()));
	w = w + n_x;
	auto len = glm::length(w);
	if (std::isnan(len) || len <= EPS5 || glm::dot(w, n_x) <= EPS5)
		return n_x;
	return glm::normalize(w);
}

glm::vec3
Distribution::sample_box(glm::vec3 x) const
{
	auto &s = primitive_->primitive_specific[0];
	glm::vec3 weight = {s.y * s.z, s.x * s.z, s.x * s.y};
	while (true) {
		float u = rnd_->uniform(0.f, weight.x + weight.y + weight.z);
		float sign = (rnd_->uniform() > 0.5f) ? 1.f : -1.f;
		glm::vec3 y;
		if (u < weight.x)
			y = glm::vec3(sign * s.x, rnd_->uniform(-s.y, s.y), rnd_->uniform(-s.z, s.z));
		else if (u < weight.x + weight.y)
			y = glm::vec3(rnd_->uniform(-s.x, s.x), sign * s.y, rnd_->uniform(-s.z, s.z));
		else
			y = glm::vec3(rnd_->uniform(-s.x, s.x), rnd_->uniform(-s.y, s.y), sign * s.z);
		y = rotate(y, conjugate(primitive_->rotation)) + primitive_->position;
		auto w = glm::normalize(y - x);
		if (primitive_->intersect(Ray{w, x}).has_value())
			return w;
	}
}

glm::vec3
Distribution::sample_ellipsoid(glm::vec3 x) const
{
	auto r = primitive_->primitive_specific[0];
	while (true) {
		auto y = r * glm::normalize(glm::vec3(rnd_->normal(), rnd_->normal(), rnd_->normal()));
		y = rotate(y, conjugate(primitive_->rotation)) + primitive_->position;
		auto w = glm::normalize(y - x);
		if (primitive_->intersect(Ray{w, x}).has_value())
			return w;
	}
}

glm::vec3
Distribution::sample_triangle(glm::vec3 x) const
{
	const auto &a = primitive_->primitive_specific[0];
	const auto &b = primitive_->primitive_specific[1] - a;
	const auto &c = primitive_->primitive_specific[2] - a;
	float u = rnd_->uniform(), v = rnd_->uniform();
	if (u + v > 1.f) {
		u = 1.f - u;
		v = 1.f - v;
	}
	auto y = primitive_->position + rotate(a + u * b + v * c, conjugate(primitive_->rotation));
	auto w = glm::normalize(y - x);
	return w;
}

glm::vec3
Distribution::sample_mixed(glm::vec3 x, glm::vec3 n_x) const
{
	auto i = (int)(rnd_->uniform(0, 1.f - EPS5) * (float)distributions_.size());
	i = std::max(0, std::min((int)distributions_.size() - 1, i));
	return distributions_[i]->sample(x, n_x);
}

glm::vec3
Distribution::sample_mixed_on_primitives(glm::vec3 x, glm::vec3 n_x) const
{
	return sample_mixed(x, n_x);
}

glm::vec3
Distribution::sample(glm::vec3 x, glm::vec3 n_x) const
{
	switch (type_) {
		case (DistributionType::COSINE):
			return sample_cosine(n_x);
		case (DistributionType::BOX):
			return sample_box(x);
		case (DistributionType::ELLIPSOID):
			return sample_ellipsoid(x);
		case (DistributionType::MIXED):
			return sample_mixed(x, n_x);
		case (DistributionType::MIXED_ON_PRIMITIVES):
			return sample_mixed_on_primitives(x, n_x);
		default:
			unreachable();
	}
}

float
Distribution::pdf1_box(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const
{
	auto &s = primitive_->primitive_specific[0];
	auto area = 8 * (s.y * s.z + s.x * s.z + s.x * s.y);
	auto w = y - x;
	auto t = glm::length(x - y);
	w = glm::normalize(w);
	if (std::isnan(glm::length(w)))
		return 0.f;
	return powf(t, 2.f) / (area * std::abs(glm::dot(w, n_y)) + EPS7);
}

float
Distribution::pdf1_ellipsoid(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const
{
	auto r = primitive_->primitive_specific[0];
	auto n = rotate(y - primitive_->position, primitive_->rotation) / r;
	auto p = 1.f / (4.f * PI * glm::length(glm::vec3(n.x * r.y * r.z, r.x * n.y * r.z, r.x * r.y * n.z)));
	auto w = y - x;
	auto t = glm::length(w);
	if (std::abs(t) < EPS4)
		return 0.f;
	w = glm::normalize(w);
	return p * powf(t, 2.f) / (std::abs(glm::dot(w, n_y)) + EPS5);
}

float
Distribution::pdf1_triangle(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const
{
	const auto &a = primitive_->primitive_specific[0];
	const auto &b = primitive_->primitive_specific[1] - a;
	const auto &c = primitive_->primitive_specific[2] - a;
	const auto normal = glm::cross(b, c);
	float p = 1.f / (0.5f * glm::length(normal));
	auto w = y - x;
	auto t = glm::length(w);
	return p * powf(t, 2.f) / (std::abs(glm::dot(w, n_y)) + EPS5);
}

float
Distribution::pdf_cosine(glm::vec3 n_x, glm::vec3 w) const
{
	return std::max(0.f, glm::dot(w, n_x) / PI);
}

float
Distribution::pdf1_primitive(glm::vec3 x, glm::vec3 w, glm::vec3 n_y) const
{
	switch (type_) {
		case (DistributionType::BOX):
			return pdf1_box(x, w, n_y);
		case (DistributionType::ELLIPSOID):
			return pdf1_ellipsoid(x, w, n_y);
		default:
			unreachable();
	}
}

float
Distribution::pdf_mixed(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const
{
	float p = 0;
	for (const auto &distrib : distributions_)
		p += distrib->pdf(x, n_x, w);
	return p / (float)distributions_.size();
}

float
Distribution::pdf_sum_bvh(uint32_t current_id, glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const
{
	Ray ray = {w, x};
	const Node &current = bvh.nodes[current_id];
	auto intersection = current.aabb.intersect(ray);
	if (!intersection.has_value())
		return 0.f;
	if (current.left_child == -1 && current.right_child == -1) {
		float pdf_sum = 0;
		for (int i = 0; i < current.primitive_count; i++)
			pdf_sum += distributions_[current.first_primitive_id + i]->pdf(x, n_x, w);
		return pdf_sum;
	}
	return pdf_sum_bvh(current.left_child, x, n_x, w) +
		pdf_sum_bvh(current.right_child, x, n_x, w);
}

float
Distribution::pdf_mixed_on_primitives(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const
{
	return pdf_sum_bvh(bvh.root, x, n_x, w) / (float)distributions_.size();
}

float
Distribution::pdf(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const
{
	switch (type_) {
		case (DistributionType::COSINE):
			return pdf_cosine(n_x, w);
		case (DistributionType::BOX):
		case (DistributionType::ELLIPSOID): {
			auto origin = x;
			float p = 0.f;
			for (int i = 0; i < 2; i++) {
				auto intersection = primitive_->intersect({w, origin});
				if (!intersection.has_value())
					return p;
				auto y = intersection.value().point;
				auto n_y = intersection.value().normal;
				p += pdf1_primitive(x, y, n_y);
				origin = intersection->point + w * EPS4;
			}
			return p;
		}
		case (DistributionType::TRIANGLE): {
			auto intersection = primitive_->intersect({w, x});
			assert(!intersection.has_value());
			auto y = intersection.value().point;
			auto n_y = intersection.value().normal;
			return pdf1_triangle(x, y, n_y);
		}
		case (DistributionType::MIXED):
			return pdf_mixed(x, n_x, w);
		case (DistributionType::MIXED_ON_PRIMITIVES):
			return pdf_mixed_on_primitives(x, n_x, w);
		default:
			unreachable();
	}
}

#ifndef RAYTRACING_RANDOM_HPP
#define RAYTRACING_RANDOM_HPP

#include "BVH.hpp"
#include <geometry_utils.hpp>
#include <Primitive.hpp>
#include <Ray.hpp>
#include <utils.hpp>

#include <chrono>
#include <iostream>
#include <memory>
#include <random>

struct Random {
	Random();

	float uniform(float l = 0.f, float r = 1.f);
	float normal(float mu = 0.f, float sigma = 1.f);

	std::minstd_rand rnd;
	std::uniform_real_distribution<float> uniform_;
	std::normal_distribution<float> normal_;
};

enum class DistributionType {
    COSINE,
    BOX,
    ELLIPSOID,
    TRIANGLE,
    MIXED,
    MIXED_ON_PRIMITIVES,
    DISTRIBUTIONS_NUMBER
};

struct Distribution {
	explicit Distribution(std::shared_ptr<Random> rnd, DistributionType type);

	void init_box(PrimitivePtr box);

	void init_ellipsoid(PrimitivePtr ellipsoid);

	void init_mixed(std::vector<std::unique_ptr<Distribution>> &&distributions);

	void init_mixed_on_primitives(const std::vector<std::unique_ptr<Distribution>> &distributions);

	glm::vec3 sample_cosine(glm::vec3 n_x) const;

	glm::vec3 sample_box(glm::vec3 x) const;

	glm::vec3 sample_ellipsoid(glm::vec3 x) const;

	glm::vec3 sample_triangle(glm::vec3 x) const;

	glm::vec3 sample_mixed(glm::vec3 x, glm::vec3 n_x) const;

	glm::vec3 sample_mixed_on_primitives(glm::vec3 x, glm::vec3 n_x) const;

    	glm::vec3 sample(glm::vec3 x, glm::vec3 n_x) const;

	float pdf1_box(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf1_ellipsoid(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf1_triangle(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf_cosine(glm::vec3 n_x, glm::vec3 w) const;

    	float pdf1_primitive(glm::vec3 x, glm::vec3 w, glm::vec3 n_y) const;

	float pdf_mixed(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	float pdf_sum_bvh(uint32_t pos, glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	float pdf_mixed_on_primitives(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	float pdf(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	std::shared_ptr<Random> rnd_;
	DistributionType type_;
	PrimitivePtr primitive_;
	std::vector<std::unique_ptr<Distribution>> distributions_;
	BVH bvh;
};

#endif //RAYTRACING_RANDOM_HPP

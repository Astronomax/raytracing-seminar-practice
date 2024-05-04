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
    	Distribution() = default;

	explicit Distribution(DistributionType type);

	void init_box(const Primitive* box);

	void init_ellipsoid(const Primitive* ellipsoid);

	void init_triangle(const Primitive* ellipsoid);

	void init_mixed(std::vector<Distribution> &&distributions);

	void init_mixed_on_primitives(const std::vector<Distribution> &distributions);

	glm::vec3 sample_cosine(Random &rnd_, glm::vec3 n_x) const;

	glm::vec3 sample_box(Random &rnd_, glm::vec3 x) const;

	glm::vec3 sample_ellipsoid(Random &rnd_, glm::vec3 x) const;

	glm::vec3 sample_triangle(Random &rnd_, glm::vec3 x) const;

	glm::vec3 sample_mixed(Random &rnd_, glm::vec3 x, glm::vec3 n_x) const;

	glm::vec3 sample_mixed_on_primitives(Random &rnd_, glm::vec3 x, glm::vec3 n_x) const;

    	glm::vec3 sample(Random &rnd_, glm::vec3 x, glm::vec3 n_x) const;

	float pdf1_box(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf1_ellipsoid(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf1_triangle(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf_cosine(glm::vec3 n_x, glm::vec3 w) const;

    	float pdf1_primitive(glm::vec3 x, glm::vec3 w, glm::vec3 n_y) const;

	float pdf_mixed(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	float pdf_sum_bvh(uint32_t pos, glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	float pdf_mixed_on_primitives(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	float pdf(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	DistributionType type_;
	const Primitive *primitive_;
	std::vector<Distribution> distributions_;
	BVH bvh;
};

#endif //RAYTRACING_RANDOM_HPP

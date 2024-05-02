#ifndef RAYTRACING_RANDOM_HPP
#define RAYTRACING_RANDOM_HPP

#include <utils.hpp>
#include <geometry_utils.hpp>
#include <Primitive.hpp>
#include <Ray.hpp>

#include <random>
#include <chrono>
#include <memory>
#include <iostream>

struct Random {
	Random();

	float uniform(float l = 0.f, float r = 1.f);
	float normal();

	std::minstd_rand rnd;
	std::uniform_real_distribution<float> uniform_;
	std::normal_distribution<float> normal_;
};

enum class DistribType {
    COSINE,
    BOX,
    ELLIPSOID,
    MIXED,
    DISTRIBS_NUMBER
};

struct Distrib {
	explicit Distrib(Random &rnd, DistribType type);

	void init_box(Primitive box);

	void init_ellipsoid(Primitive ellipsoid);

	void init_mixed(std::vector<std::unique_ptr<Distrib>> &&distribs_);

	glm::vec3 sample_cosine(glm::vec3 n_x) const;

	glm::vec3 sample_box(glm::vec3 x) const;

	glm::vec3 sample_ellipsoid(glm::vec3 x) const;

	glm::vec3 sample_mixed(glm::vec3 x, glm::vec3 n_x) const;

    	glm::vec3 sample(glm::vec3 x, glm::vec3 n_x) const;

	float pdf1_box(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf1_ellipsoid(glm::vec3 x, glm::vec3 y, glm::vec3 n_y) const;

	float pdf_cosine(glm::vec3 n_x, glm::vec3 w) const;

    	float pdf1_primitive(glm::vec3 x, glm::vec3 w, glm::vec3 n_y) const;

	float pdf_mixed(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	float pdf(glm::vec3 x, glm::vec3 n_x, glm::vec3 w) const;

	Random &rnd;
	DistribType type;
	Primitive primitive;
	std::vector<std::unique_ptr<Distrib>> distribs;
};

#endif //RAYTRACING_RANDOM_HPP

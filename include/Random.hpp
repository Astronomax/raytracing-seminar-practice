#ifndef RAYTRACING_RANDOM_HPP
#define RAYTRACING_RANDOM_HPP

#include <random>
#include <chrono>

class Random {
public:
    Random() : rnd(std::chrono::steady_clock::now().time_since_epoch().count()),
	       uniform_(0.f, 1.f), normal_(0.f, 1.f) {
    }

    float uniform() {
	    return uniform_(rnd);
    }

    float normal() {
	    return normal_(rnd);
    }

private:
    std::minstd_rand rnd;
    std::uniform_real_distribution<float> uniform_;
    std::normal_distribution<float> normal_;
};

#endif //RAYTRACING_RANDOM_HPP

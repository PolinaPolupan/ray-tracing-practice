//
// Created by polup on 13/01/2026.
//

#ifndef SAMPLING_H
#define SAMPLING_H
#include <random>

#include "math.h"


class sampler {
public:
    virtual ~sampler() = default;

    virtual double gen_1d() = 0;
    virtual point2 gen_2d() = 0;
};

class independent_sampler: public sampler {
public:
    double gen_1d() override {
        std::uniform_real_distribution<double> distribution(0.0, 1.0);
        return distribution(generator);
    }

    point2 gen_2d() override {
        return point2(gen_1d(), gen_1d());
    }

private:
    std::mt19937 generator;
};

inline vec3 sample_uniform_hemisphere(const point2 u) {
    const auto r1 = u.x;
    const auto r2 = u.y;

    const auto phi = 2*pi*r1;
    auto x = std::cos(phi) * std::sqrt(r2);
    auto y = std::sin(phi) * std::sqrt(r2);
    auto z = std::sqrt(1-r2);

    return {x, y, z};
}

[[nodiscard]] vec3 sample_square_stratified(point2 u, int s_i, int s_j, double recip_sqrt_spp);

inline point3 defocus_disk_sample(
    const std::shared_ptr<sampler>& samp,
    const point3& center,
    const vec3& du,
    const vec3& dv
) {
    vec3 p;
    do {
        p = vec3(
            samp->gen_1d() * 2 - 1,
            samp->gen_1d() * 2 - 1,
            0
        );
    } while (p.length_squared() >= 1);

    return center + p.x() * du + p.y() * dv;
}

inline vec3 random_to_sphere(const point2 u, const double radius, const double distance_squared) {
    const auto r1 = u.x;
    const auto r2 = u.y;
    auto z = 1 + r2*(std::sqrt(1-radius*radius/distance_squared) - 1);

    const auto phi = 2*pi*r1;
    auto x = std::cos(phi) * std::sqrt(1-z*z);
    auto y = std::sin(phi) * std::sqrt(1-z*z);

    return {x, y, z};
}

#endif //SAMPLING_H

//
// Created by polup on 13/01/2026.
//

#ifndef SAMPLING_H
#define SAMPLING_H
#include "Vec3.h"


class sampler {
public:
    virtual ~sampler() = default;

    virtual double gen_1d() = 0;
    virtual double gen_2d() = 0;
};

class independent_sampler: public sampler {
public:
    double gen_1d() override {
        std::uniform_real_distribution<double> distribution(0.0, 1.0);
        return distribution(generator);
    }

private:
    std::mt19937 generator;
};

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(const double min, const double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

inline int random_int(const int min, const int max) {
    // Returns a random integer in [min,max].
    return static_cast<int>(random_double(min, max + 1));
}

inline Vec3 random_in_unit_disk() {
    while (true) {
        auto p = Vec3(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

inline Vec3 sample_uniform_hemisphere() {
    const auto r1 = random_double();
    const auto r2 = random_double();

    const auto phi = 2*pi*r1;
    auto x = std::cos(phi) * std::sqrt(r2);
    auto y = std::sin(phi) * std::sqrt(r2);
    auto z = std::sqrt(1-r2);

    return {x, y, z};
}

inline Vec3 random(const double min, const double max) {
    return {random_double(min,max), random_double(min,max), random_double(min,max)};
}

[[nodiscard]] Vec3 sample_square_stratified(int s_i, int s_j, double recip_sqrt_spp);

[[nodiscard]] inline Vec3 sample_square() {
    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    return {random_double() - 0.5, random_double() - 0.5, 0};
}

[[nodiscard]] inline Point3 defocus_disk_sample(Point3 center, Vec3 defocus_disk_u, Vec3 defocus_disk_v) {
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}

#endif //SAMPLING_H

//
// Created by polup on 13/01/2026.
//

#ifndef SAMPLING_H
#define SAMPLING_H
#include <random>

#include "math.h"


class sampler {
public:
    explicit sampler(const int spp) : spp_(spp) {}
    virtual ~sampler() = default;

    virtual void start_pixel() = 0;
    virtual bool start_next_sample() = 0;
    virtual double gen_1d() = 0;
    virtual point2d gen_2d() = 0;

    int get_spp() const { return spp_; }

protected:
    int spp_;
};

class independent_sampler: public sampler {
public:
    explicit independent_sampler(const int spp) : sampler(spp) {}

    void start_pixel() override {
        sample_index = 0;
    }

    bool start_next_sample() override {
        return sample_index++ < spp_;
    }

    double gen_1d() override {
        std::uniform_real_distribution<double> distribution(0.0, 1.0);
        return distribution(generator);
    }

    point2d gen_2d() override {
        return point2(gen_1d(), gen_1d());
    }

private:
    int sample_index = 0;
    std::mt19937 generator;
};

class stratified_sampler final : public sampler {
public:
    explicit stratified_sampler(const int spp): sampler(spp), sqrt_spp(static_cast<int>(std::sqrt(spp))) {}

    void start_pixel() override {
        sample_index = 0;
    }

    bool start_next_sample() override {
        return sample_index++ < spp_;
    }

    double gen_1d() override {
        return rng01();
    }

    point2d gen_2d() override {
        const int stratum = sample_index;
        const int x = stratum % sqrt_spp;
        const int y = stratum / sqrt_spp;

        return {
            (x + rng01()) / sqrt_spp,
            (y + rng01()) / sqrt_spp
        };
    }

private:
    double rng01() {
        return std::uniform_real_distribution<double>(0, 1)(rng);
    }

    int sqrt_spp;
    int sample_index = 0;
    std::mt19937 rng;
};

vec3 sample_uniform_hemisphere(point2d u);

vec3 sample_uniform_sphere(const point2d& u);

point3 defocus_disk_sample(const std::shared_ptr<sampler>& samp, const point3& center, const vec3& du, const vec3& dv);

vec3 random_to_sphere(point2d u, double radius, double distance_squared);

vec3 cosine_sample_hemisphere(const point2d& u);


#endif //SAMPLING_H

//
// Created by polup on 13/01/2026.
//

#ifndef SAMPLING_H
#define SAMPLING_H
#include <random>

#include "math.h"


class sampler
{
public:
    explicit sampler(const int spp) : _spp(spp) {}
    virtual ~sampler() = default;

    virtual void start_pixel() = 0;
    virtual bool start_next_sample() = 0;
    virtual double gen_1d() = 0;
    virtual point2d gen_2d() = 0;
    [[nodiscard]] virtual std::unique_ptr<sampler> clone() const = 0;

    [[nodiscard]] int get_spp() const { return _spp; }

protected:
    int _spp;
};

class independent_sampler: public sampler
{
public:
    explicit independent_sampler(const int spp) : sampler(spp) {}

    [[nodiscard]] std::unique_ptr<sampler> clone() const override {
        return std::make_unique<independent_sampler>(*this);
    }

    void start_pixel() override {
        sample_index = 0;
    }

    bool start_next_sample() override {
        return sample_index++ < _spp;
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

class stratified_sampler final : public sampler
{
public:
    explicit stratified_sampler(const int spp): sampler(spp), _sqrt_spp(static_cast<int>(std::sqrt(spp))) {}

    [[nodiscard]] std::unique_ptr<sampler> clone() const override {
        return std::make_unique<stratified_sampler>(*this);
    }

    void start_pixel() override {
        _sample_index = 0;
    }

    bool start_next_sample() override {
        return _sample_index++ < _spp;
    }

    double gen_1d() override {
        return rng01();
    }

    point2d gen_2d() override {
        const int stratum = _sample_index - 1;
        const int x = stratum % _sqrt_spp;
        const int y = stratum / _sqrt_spp;

        return {
            (x + rng01()) / _sqrt_spp,
            (y + rng01()) / _sqrt_spp
        };
    }

private:
    double rng01() { return _dist(_rng); }

    std::mt19937 _rng;
    std::uniform_real_distribution<double> _dist{0.0, 1.0};

    int _sqrt_spp;
    int _sample_index = 0;
};

vec3 sample_uniform_hemisphere(point2d u);

vec3 sample_uniform_sphere(const point2d& u);

point3 defocus_disk_sample(sampler& samp, const point3& center, const vec3& du, const vec3& dv);

vec3 random_to_sphere(point2d u, double radius, double distance_squared);

vec3 cosine_sample_hemisphere(const point2d& u);


#endif //SAMPLING_H

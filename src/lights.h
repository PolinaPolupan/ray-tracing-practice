#ifndef LIGHTS_H
#define LIGHTS_H
#include "math.h"

struct light_li_sample
{
    color li;
    vec3d wi;
    vec3d p_light;
    double pdf{};
};

class light
{
public:
    virtual ~light() = default;
    light() = default;
    explicit light(const vec3d& pos): pos_(pos) {}
    virtual light_li_sample sample_Li(const vec3d& p, const point2d& u) = 0;
    [[nodiscard]] virtual double pdf_Li() const = 0;
    virtual double Le() = 0;
    virtual bool is_infinite() = 0;

protected:
    vec3d pos_;
};

class point_light: public light
{
public:
    point_light(const vec3d& pos, const double scale): light(pos), scale_(scale) {}

    light_li_sample sample_Li(const vec3d& p, const point2d& u) override
    {
        const vec3d d = pos_ - p;
        const vec3d wi = unit_vector(d);

        const double dist2 = std::max(0.001, d.length_squared());
        const color li = (scale_ * vec3d(1.0f)) / dist2;

        return {li, wi, pos_, 1.0};
    }

    [[nodiscard]] double pdf_Li() const override { return 0.0; }

    double Le() override { return scale_; }

    bool is_infinite() override { return false; }

private:
    double scale_;
};

class uniform_infinite_light : public light
{
public:
    uniform_infinite_light(const bounds3d& scene_bounds, const double scale) : scale_(scale)
    {
        center_ = (scene_bounds.p_min + scene_bounds.p_max) / 2;
        radius_ = (scene_bounds.p_max - center_).length();
    }

    light_li_sample sample_Li(const vec3d& p, const point2d& u) override
    {
        // Sample a random direction on the sphere
        const vec3d wi = sample_uniform_sphere(u);

        // Position should be infinitely far in the direction of wi
        const point3d light_pos = p + wi * (2 * radius_);

        return {scale_, wi, light_pos, 1 / (4 * pi)};
    }

    [[nodiscard]] double pdf_Li() const override { return 1 / (4 * pi); }

    bool is_infinite() override { return true; }

    double Le() override { return scale_; }

private:
    double scale_;
    point3d center_;
    double radius_;
};

struct sampled_light
{
    std::shared_ptr<light> light_ptr;
    double p;
};

class light_sampler
{
public:
    explicit light_sampler(const std::vector<std::shared_ptr<light>>& lights): lights_(lights) {};

    [[nodiscard]] sampled_light sample(const double u) const
    {
        if (lights_.empty()) return {};

        const int idx = std::min<int>(u * lights_.size(), lights_.size() - 1);
        return {lights_[idx], 1.0 / lights_.size()};
    }

private:
    std::vector<std::shared_ptr<light>> lights_;
};

#endif //LIGHTS_H
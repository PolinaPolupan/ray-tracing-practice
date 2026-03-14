#ifndef LIGHTS_H
#define LIGHTS_H
#include "math.h"

struct light_li_sample
{
    color li;
    vec3 wi;
    vec3 p_light;
    double pdf{};
};

class light
{
public:
    virtual ~light() = default;
    light() = default;
    explicit light(const vec3& pos): pos_(pos) {}
    virtual light_li_sample sample_li(const vec3& p, const point2d& u) = 0;
    [[nodiscard]] virtual double pdf_li() const = 0;

protected:
    vec3 pos_;
};

class point_light: public light
{
public:
    point_light(const vec3& pos, const double scale): light(pos), scale_(scale) {}

    light_li_sample sample_li(const vec3& p, const point2d& u) override
    {
        const vec3 d = pos_ - p;
        const vec3 wi = unit_vector(d);

        const double dist2 = std::max(0.001, d.length_squared());
        const color li = (scale_ * vec3(1.0f)) / dist2;

        return {li, wi, pos_, 1.0};
    }

    [[nodiscard]] double pdf_li() const override { return 0.0; }

private:
    double scale_;
};

class uniform_infinite_light : public light
{
public:
    uniform_infinite_light(bounds3 scene_bounds)
    {

    }

    light_li_sample sample_li(const vec3& p, const point2d& u) override
    {
        const vec3 wi = sample_uniform_sphere(u);

        return {scale_, wi, p + wi * 2 * radius_ , 1 / (4 * pi)};
    }

    double pdf_li() const override { return 1 / (4 * pi); }

private:
    double scale_;
    point3 center_;
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
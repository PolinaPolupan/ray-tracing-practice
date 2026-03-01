#ifndef RAY_TRACING_IN_ONE_WEEK_LIGHTS_H
#define RAY_TRACING_IN_ONE_WEEK_LIGHTS_H
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

    explicit light(const vec3& pos): pos(pos) {}

    virtual light_li_sample sample_li(const vec3& p) = 0;

    [[nodiscard]] virtual double pdf_li() const = 0;

protected:
    vec3 pos;
};

class point_light: public light
{
public:
    point_light(const vec3& pos, const double scale): light(pos), scale(scale) {}

    light_li_sample sample_li(const vec3& p) override
    {
        const vec3 d = pos - p;
        const vec3 wi = unit_vector(d);

        const double dist2 = std::max(0.001, d.length_squared());
        const color li = (scale * vec3(1.0f)) / dist2;

        return {li, wi, pos, 1.0};
    }

    [[nodiscard]] double pdf_li() const override
    {
        return 0.0;
    }

private:
    double scale;
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

#endif //RAY_TRACING_IN_ONE_WEEK_LIGHTS_H
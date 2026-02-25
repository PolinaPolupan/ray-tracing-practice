#ifndef RAY_TRACING_IN_ONE_WEEK_BSDF_H
#define RAY_TRACING_IN_ONE_WEEK_BSDF_H
#include "math.h"
#include "sampling.h"

struct bsdf_sample {
    vec3 wi;        // sampled direction
    color f;        // BSDF value
    double pdf{};   // probability of wi
};

class bsdf
{
public:
    explicit bsdf(const vec3& n): frame_(n) {}

    virtual ~bsdf() = default;

    [[nodiscard]] virtual bsdf_sample sample_f(const vec3& wo_world, const point2& u) const = 0;

    [[nodiscard]] virtual color f(const vec3& wo_world, const vec3& wi_world) const = 0;

    [[nodiscard]] virtual double pdf(const vec3& wo_world, const vec3& wi_world) const = 0;

    [[nodiscard]] virtual bool is_specular() const { return false; }

protected:
    frame frame_;
};

class lambertian_bsdf final: public bsdf {
public:
    lambertian_bsdf(const color& albedo, const vec3& normal): bsdf(normal), albedo_(albedo) {}

    [[nodiscard]] bsdf_sample sample_f(const vec3& wo_world, const point2& u) const override;

    [[nodiscard]] color f(const vec3& wo_world, const vec3& wi_world) const override
    {
        const vec3 wi = frame_.to_local(wi_world);

        if (wi.z() <= 0) return {0,0,0};

        return albedo_ / pi;
    }

    [[nodiscard]] double pdf(const vec3& wo_world, const vec3& wi_world) const override
    {
        const vec3 wi = frame_.to_local(wi_world);

        if (wi.z() <= 0) return 0;

        return wi.z() / pi;
    }

private:
    color albedo_;
};

class dielectric_bsdf final : public bsdf {
public:
    dielectric_bsdf(const double ior, const vec3& normal, const bool front_face):
    bsdf(normal), ior_(ior), front_face_(front_face) {}

    [[nodiscard]] bool is_specular() const override { return true; }

    [[nodiscard]] bsdf_sample sample_f(const vec3& wo_world, const point2& u) const override;

    [[nodiscard]] double pdf(const vec3& wo, const vec3& wi) const override {
        return 0;
    }

    [[nodiscard]] color f(const vec3& wo, const vec3& wi) const override {
        return {0, 0, 0};
    }

private:
    double ior_;
    bool front_face_;

    static double reflectance(const double cosine, const double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};

class metal_bsdf final : public bsdf {
public:
    metal_bsdf(const color& albedo, const double fuzz, const vec3& normal)
        : bsdf(normal), albedo_(albedo), fuzz_(fuzz) {}

    [[nodiscard]] bool is_specular() const override { return true; }

    [[nodiscard]] bsdf_sample sample_f(const vec3& wo_world, const point2& u) const override;

    [[nodiscard]] color f(const vec3& wo, const vec3& wi) const override { return {0,0,0}; }
    [[nodiscard]] double pdf(const vec3& wo, const vec3& wi) const override { return 0.0; }

private:
    color albedo_;
    double fuzz_;
};

#endif //RAY_TRACING_IN_ONE_WEEK_BSDF_H
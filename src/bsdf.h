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
    virtual ~bsdf() = default;

    [[nodiscard]] virtual bsdf_sample sample_f(const vec3& wo, const point2& u) const = 0;

    [[nodiscard]] virtual color f(const vec3& wo, const vec3& wi) const = 0;

    [[nodiscard]] virtual double pdf(const vec3& wo, const vec3& wi) const = 0;

    [[nodiscard]] virtual bool is_specular() const { return false; }
};

class lambertian_bsdf final: public bsdf {
public:
    lambertian_bsdf(const color& albedo, const vec3& normal)
      : albedo(albedo), n(normal) {}

    [[nodiscard]] bsdf_sample sample_f(const vec3& wo, const point2& u) const override;

    [[nodiscard]] color f(const vec3&, const vec3& wi) const override {
        double cos = dot(n, unit_vector(wi));
        if (cos <= 0) return {0,0,0};
        return albedo / pi;
    }

    [[nodiscard]] double pdf(const vec3&, const vec3& wi) const override {
        const double cos = dot(n, unit_vector(wi));
        return cos > 0 ? cos / pi : 0;
    }

private:
    color albedo;
    vec3 n;
};

class dielectric_bsdf final : public bsdf {
public:
    dielectric_bsdf(const double ior, const vec3& normal, const bool front_face): ior_(ior), n_(normal), front_face_(front_face) {}

    [[nodiscard]] bool is_specular() const override { return true; }

    [[nodiscard]] bsdf_sample sample_f(const vec3& wo, const point2& u) const override;

    [[nodiscard]] double pdf(const vec3& wo, const vec3& wi) const override {
        return 0;
    }

    [[nodiscard]] color f(const vec3& wo, const vec3& wi) const override {
        return {0, 0, 0};
    }

private:
    double ior_;
    vec3 n_;
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
        : albedo(albedo), fuzz(fuzz), n(normal) {}

    [[nodiscard]] bool is_specular() const override { return true; }

    [[nodiscard]] bsdf_sample sample_f(const vec3& wo, const point2& u) const override;

    [[nodiscard]] color f(const vec3& wo, const vec3& wi) const override { return {0,0,0}; }
    [[nodiscard]] double pdf(const vec3& wo, const vec3& wi) const override { return 0.0; }

private:
    color albedo;
    double fuzz;
    vec3 n;
};

#endif //RAY_TRACING_IN_ONE_WEEK_BSDF_H
//
// Created by polup on 19/01/2026.
//

#ifndef RAY_TRACING_IN_ONE_WEEK_BSDF_H
#define RAY_TRACING_IN_ONE_WEEK_BSDF_H
#include "math.h"
#include "sampling.h"

struct bsdf_sample {
    vec3 wi;      // sampled direction
    color f; // BSDF value
    double pdf;        // probability of wi
};

class bsdf
{
public:
    virtual ~bsdf() = default;

    virtual bsdf_sample sample_f(const vec3& wo, const point2& u) const = 0;

    virtual color f(const vec3& wo, const vec3& wi) const = 0;

    virtual double pdf(const vec3& wo, const vec3& wi) const = 0;

    [[nodiscard]] virtual bool is_specular() const { return false; }
};

class lambertian_bsdf final: public bsdf {
public:
    lambertian_bsdf(const color& albedo, const vec3& normal)
      : albedo(albedo), n(normal) {}

    bsdf_sample sample_f(const vec3& wo, const point2& u) const override {
        // Build local frame
        orthonormal_base onb(n);

        // Sample in local space
        vec3 wi_local = cosine_sample_hemisphere(u);
        vec3 wi_world = onb.transform(wi_local);

        double pdf_val = pdf(wo, wi_world);
        color f_val = f(wo, wi_world);

        return { wi_world, f_val, pdf_val };
    }

    color f(const vec3&, const vec3& wi) const override {
        double cos = dot(n, unit_vector(wi));
        if (cos <= 0) return {0,0,0};
        return albedo / pi;
    }

    double pdf(const vec3&, const vec3& wi) const override {
        const double cos = dot(n, unit_vector(wi));
        return cos > 0 ? cos / pi : 0;
    }

private:
    color albedo;
    vec3 n;
};



#endif //RAY_TRACING_IN_ONE_WEEK_BSDF_H
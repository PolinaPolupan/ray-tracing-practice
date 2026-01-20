//
// Created by polup on 19/01/2026.
//

#include "bsdf.h"

bsdf_sample lambertian_bsdf::sample_f(const vec3& wo, const point2& u) const
{
    // Build local frame
    const orthonormal_base onb(n);

    // Sample in local space
    const vec3 wi_local = cosine_sample_hemisphere(u);
    const vec3 wi_world = onb.transform(wi_local);

    const double pdf_val = pdf(wo, wi_world);
    const color f_val = f(wo, wi_world);

    return { wi_world, f_val, pdf_val };
}

bsdf_sample dielectric_bsdf::sample_f(const vec3& wo, const point2& u) const
{
    // NOTE: 'wo' is the vector pointing OUT from the surface towards the camera.
    // The original logic used 'unit_direction' (incident ray), which is -wo.

    const double ri = front_face_ ? (1.0 / ior_) : ior_;

    // Incident vector (pointing INTO the surface)
    const vec3 wi_in = -wo;

    const double cos_theta = std::fmin(dot(wo, n_), 1.0);
    const double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

    const bool cannot_refract = ri * sin_theta > 1.0;
    vec3 direction;

    // u.x is used as the random seed for the fresnel check
    if (cannot_refract || reflectance(cos_theta, ri) > u.x) {
        direction = reflect(wi_in, n_);
    } else {
        direction = refract(wi_in, n_, ri);
    }

    // For perfect specular, the "f" value is effectively 1.0 (white)
    // because the PDF is a delta function. The integrator handles the recursion.
    return { direction, color(1.0, 1.0, 1.0), 1.0 };
}

bsdf_sample metal_bsdf::sample_f(const vec3& wo, const point2& u) const
{
    // 1. Calculate perfect reflection
    // wo points OUT, so -wo points IN
    const vec3 reflected = reflect(-wo, n);

    // 2. Add fuzz
    // We need to map the 2D sample 'u' to a random point on a unit sphere.
    // (If fuzz is 0, this part is skipped effectively)
    const vec3 fuzz_vec = sample_uniform_sphere(u);
    const vec3 wi = unit_vector(reflected + fuzz * fuzz_vec);

    // 3. Check if the fuzzed ray scattered below the surface
    if (dot(wi, n) <= 0) {
        // Absorbed
        return { vec3(0,0,0), color(0,0,0), 0.0 };
    }

    // For specular/delta interactions, we return the albedo as the throughput 'f'
    // and PDF 1.0 (placeholder).
    return { wi, albedo, 1.0 };
}

#include "bsdf.h"

bsdf_sample lambertian_bsdf::sample_f(const vec3& wo_world, const point2& u) const
{
    const vec3 wi_local = cosine_sample_hemisphere(u);

    const double pdf = wi_local.z() / pi;
    const color f = albedo_ / pi;

    return { unit_vector(frame_.from_local(wi_local)), f, pdf };
}

bsdf_sample dielectric_bsdf::sample_f(const vec3& wo_world, const point2& u) const
{
    const vec3 wo = frame_.to_local(wo_world);
    const vec3 n(0,0,1);

    double eta_i = 1.0;
    double eta_t = ior_;

    const bool entering = wo.z() > 0;

    if (!entering)
        std::swap(eta_i, eta_t);

    const double eta = eta_i / eta_t;

    const vec3 wi_in = -wo;

    const double cos_theta = std::abs(wo.z());
    const double sin_theta = std::sqrt(std::max(0.0, 1 - cos_theta*cos_theta));

    const bool cannot_refract = eta * sin_theta > 1.0;

    const double Fr = reflectance(cos_theta, eta);

    vec3 wi_local;

    if (cannot_refract || u.x < Fr) {
        // reflect
        wi_local = reflect(wi_in, n);
    } else {
        // refract
        wi_local = refract(wi_in, n, eta);
    }

    return {unit_vector(frame_.from_local(wi_local)), color(1.0),1.0 };
}

bsdf_sample metal_bsdf::sample_f(const vec3& wo_world, const point2& u) const
{
    const vec3 wo = frame_.to_local(wo_world);

    const vec3 n(0,0,1);

    const vec3 wi_local = reflect(-wo, n);

    if (wi_local.z() <= 0)
        return { vec3(0), color(0), 0 };

    return {unit_vector(frame_.from_local(wi_local)), albedo_, 1.0};
}

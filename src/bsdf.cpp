#include "bsdf.h"

bsdf_sample lambertian_bsdf::sample_f(const vec3d& wo_world, const point2d& u) const
{
    const vec3d wi_local = cosine_sample_hemisphere(u);

    const double pdf = wi_local.z() / pi;
    const color f = albedo_ / pi;

    return { unit_vector(frame_.from_local(wi_local)), f, pdf };
}

bsdf_sample dielectric_bsdf::sample_f(const vec3d& wo_world, const point2d& u) const
{
    const vec3d wo = frame_.to_local(wo_world);
    const vec3d n(0, 0, 1);

    double eta_i = 1.0;
    double eta_t = ior_;
    if (!front_face_) std::swap(eta_i, eta_t);
    double eta = eta_i / eta_t;

    const vec3d wi_in = -wo;

    double cos_theta = std::abs(wo.z());
    const double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta*cos_theta));

    const bool cannot_refract = eta * sin_theta > 1.0;

    double schlick_cosine = cos_theta;
    if (eta_i > eta_t && !cannot_refract) {
        schlick_cosine = std::sqrt(1.0 - eta * eta * (1.0 - cos_theta * cos_theta));
    }

    const double Fr = reflectance(schlick_cosine, ior_);

    vec3d wi_local;
    if (cannot_refract || u.x < Fr) {
        wi_local = reflect(wi_in, n);
    } else {
        wi_local = refract(wi_in, n, eta);
    }

    return {unit_vector(frame_.from_local(wi_local)), color(1.0), 1.0 };
}

bsdf_sample metal_bsdf::sample_f(const vec3d& wo_world, const point2d& u) const
{
    const vec3d wo = frame_.to_local(wo_world);

    const vec3d n(0,0,1);

    const vec3d wi_local = reflect(-wo, n);

    if (wi_local.z() <= 0)
        return { vec3d(0), color(0), 0 };

    return {unit_vector(frame_.from_local(wi_local)), albedo_, 1.0};
}

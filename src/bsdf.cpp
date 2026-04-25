#include "bsdf.h"

bsdf_sample LambertianBsdf::sample_f(const vec3d&  /*wo_world*/, const point2d& u) const
{
    const vec3d wi_local = cosine_sample_hemisphere(u);

    const double pdf = wi_local.z() / pi;
    const color f = albedo_ / pi;

    return { .wi_=unit_vector(frame_.from_local(wi_local)), .f_=f, .pdf_=pdf };
}

bsdf_sample DielectricBsdf::sample_f(const vec3d& wo_world, const point2d& u) const
{
    const vec3d wo = frame_.to_local(wo_world);
    const vec3d n(0, 0, 1);

    double eta_i = 1.0;
    double eta_t = ior_;
    if (!front_face_) { std::swap(eta_i, eta_t);
}
    double const eta = eta_i / eta_t;

    const vec3d wi_in = -wo;

    double const cos_theta = std::abs(wo.z());
    const double sin_theta = std::sqrt(std::max(0.0, 1.0 - (cos_theta*cos_theta)));

    const bool cannot_refract = eta * sin_theta > 1.0;

    double schlick_cosine = cos_theta;
    if (eta_i > eta_t && !cannot_refract) {
        schlick_cosine = std::sqrt(1.0 - (eta * eta * (1.0 - cos_theta * cos_theta)));
    }

    const double fr = reflectance(schlick_cosine, ior_);

    vec3d wi_local;
    if (cannot_refract || u.x < fr) {
        wi_local = reflect(wi_in, n);
    } else {
        wi_local = refract(wi_in, n, eta);
    }

    return {.wi_=unit_vector(frame_.from_local(wi_local)), .f_=color(1.0), .pdf_=1.0 };
}

bsdf_sample MetalBsdf::sample_f(const vec3d& wo_world, const point2d&  /*u*/) const
{
    const vec3d wo = frame_.to_local(wo_world);

    const vec3d n(0,0,1);

    const vec3d wi_local = reflect(-wo, n);

    if (wi_local.z() <= 0) {
        return { .wi_=vec3d(0), .f_=color(0), .pdf_=0 };
}

    return {.wi_=unit_vector(frame_.from_local(wi_local)), .f_=albedo_, .pdf_=1.0};
}

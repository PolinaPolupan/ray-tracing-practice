//
// Created by polup on 13/01/2026.
//

#include "sampling.h"

vec3 sample_uniform_hemisphere(const point2 u)
{
    const double z = u.x;
    const double r = std::sqrt(std::max(0.0, 1.0 - z * z));
    const double phi = 2.0 * pi * u.y;

    return { r * std::cos(phi), r * std::sin(phi), z };
}

vec3 sample_uniform_sphere(const point2& u)
{
    const double z = 1.0 - 2.0 * u.x;
    const double r = std::sqrt(std::max(0.0, 1.0 - z * z));
    const double phi = 2.0 * pi * u.y;
    auto x = r * std::cos(phi);
    auto y = r * std::sin(phi);

    return { x, y, z };
}

point3 defocus_disk_sample(const std::shared_ptr<sampler>& samp, const point3& center, const vec3& du, const vec3& dv)
{
    vec3 p;
    do {
        p = vec3(
            samp->gen_1d() * 2 - 1,
            samp->gen_1d() * 2 - 1,
            0
        );
    } while (p.length_squared() >= 1);

    return center + p.x() * du + p.y() * dv;
}

vec3 random_to_sphere(const point2 u, const double radius, const double distance_squared)
{
    const auto r1 = u.x;
    const auto r2 = u.y;
    auto z = 1 + r2*(std::sqrt(1-radius*radius/distance_squared) - 1);

    const auto phi = 2*pi*r1;
    auto x = std::cos(phi) * std::sqrt(1-z*z);
    auto y = std::sin(phi) * std::sqrt(1-z*z);

    return {x, y, z};
}

vec3 cosine_sample_hemisphere(const point2& u)
{
    const double r = std::sqrt(u.x);
    const double phi = 2 * pi * u.y;

    double x = r * std::cos(phi);
    double y = r * std::sin(phi);
    double z = std::sqrt(1 - u.x);

    return { x, y, z };
}

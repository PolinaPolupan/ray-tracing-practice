#include "math.h"

#include "ray.h"

vec3 refract(const vec3& uv, const vec3& n, const double etai_over_etat) {
    const double cos_theta = std::fmin(dot(-uv, n), 1.0);
    const vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
    const vec3 r_out_parallel =
        -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

bool bounds3::intersect(const ray& r, interval ray_t) const {
    for (int a = 0; a < 3; a++) {
        const double invD = 1.0 / r.d()[a];
        double t0 = (axis_interval(a).min - r.o()[a]) * invD;
        double t1 = (axis_interval(a).max - r.o()[a]) * invD;
        if (invD < 0.0) std::swap(t0, t1);
        ray_t.min = std::max(t0, ray_t.min);
        ray_t.max = std::min(t1, ray_t.max);
        if (ray_t.max <= ray_t.min) return false;
    }
    return true;
}

void bounds3::padToMinimums() {
    constexpr double delta = 0.0001;
    if (x.size() < delta) x = x.expand(delta);
    if (y.size() < delta) y = y.expand(delta);
    if (z.size() < delta) z = z.expand(delta);
}

const bounds3 bounds3::empty =
    bounds3(interval::empty, interval::empty, interval::empty);

const bounds3 bounds3::universe =
    bounds3(interval::universe, interval::universe, interval::universe);

const interval interval::empty    = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

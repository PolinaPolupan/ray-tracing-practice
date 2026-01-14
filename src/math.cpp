#include "math.h"

#include "ray.h"

/* ---------------- vec3d ---------------- */

vec3d::vec3d() : e{0,0,0} {}
vec3d::vec3d(const double e0, const double e1, const double e2) : e{e0,e1,e2} {}

vec3d vec3d::operator-() const {
    return {-e[0], -e[1], -e[2]};
}

vec3d& vec3d::operator+=(const vec3d& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
}

vec3d& vec3d::operator*=(const double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
}

vec3d operator*(const vec3d& u, const vec3d& v) {
    return {
        u.e[0] * v.e[0],
        u.e[1] * v.e[1],
        u.e[2] * v.e[2]
    };
}


vec3d& vec3d::operator/=(const double t) {
    return *this *= 1 / t;
}

double vec3d::length() const {
    return std::sqrt(length_squared());
}

double vec3d::length_squared() const {
    return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
}

bool vec3d::near_zero() const {
    constexpr double s = 1e-8;
    return std::fabs(e[0]) < s && std::fabs(e[1]) < s && std::fabs(e[2]) < s;
}

/* ---------------- free functions ---------------- */

std::ostream& operator<<(std::ostream& out, const vec3d& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

vec3d operator+(const vec3d& u, const vec3d& v) {
    return {u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]};
}

vec3d operator-(const vec3d& u, const vec3d& v) {
    return {u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]};
}

vec3d operator*(const double t, const vec3d& v) {
    return {t*v.e[0], t*v.e[1], t*v.e[2]};
}

vec3d operator*(const vec3d& v, const double t) {
    return t * v;
}

vec3d operator/(const vec3d& v, const double t) {
    return (1/t) * v;
}

double dot(const vec3d& u, const vec3d& v) {
    return u.e[0]*v.e[0] + u.e[1]*v.e[1] + u.e[2]*v.e[2];
}

vec3d cross(const vec3d& u, const vec3d& v) {
    return {
        u.e[1]*v.e[2] - u.e[2]*v.e[1],
        u.e[2]*v.e[0] - u.e[0]*v.e[2],
        u.e[0]*v.e[1] - u.e[1]*v.e[0]
    };
}

vec3d unit_vector(const vec3d& v) {
    return v / v.length();
}

vec3d reflect(const vec3d& v, const vec3d& n) {
    return v - 2*dot(v,n)*n;
}

vec3d refract(const vec3d& uv, const vec3d& n, const double etai_over_etat) {
    double cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3d r_out_perp = etai_over_etat * (uv + cos_theta*n);
    vec3d r_out_parallel =
        -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

/* ---------------- interval ---------------- */

const interval interval::empty    = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

interval::interval() : min(+infinity), max(-infinity) {}
interval::interval(const double min, const double max) : min(min), max(max) {}
interval::interval(const interval& a, const interval& b)
    : min(std::min(a.min, b.min)), max(std::max(a.max, b.max)) {}

double interval::size() const { return max - min; }
bool interval::contains(const double x) const { return min <= x && x <= max; }
bool interval::surrounds(const double x) const { return min < x && x < max; }

double interval::clamp(const double x) const {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

interval interval::expand(const double delta) const {
    const double p = delta * 0.5;
    return {min - p, max + p};
}

interval operator+(const interval& i, const double d) {
    return {i.min + d, i.max + d};
}

interval operator+(double d, const interval& i) {
    return i + d;
}

/* ---------------- bounds3d ---------------- */

const bounds3d bounds3d::empty =
    bounds3d(interval::empty, interval::empty, interval::empty);

const bounds3d bounds3d::universe =
    bounds3d(interval::universe, interval::universe, interval::universe);

bounds3d::bounds3d() = default;

bounds3d::bounds3d(const interval& x_, const interval& y_, const interval& z_)
    : x(x_), y(y_), z(z_) {
    padToMinimums();
}

bounds3d::bounds3d(const point3d& a, const point3d& b) {
    x = interval(std::min(a[0], b[0]), std::max(a[0], b[0]));
    y = interval(std::min(a[1], b[1]), std::max(a[1], b[1]));
    z = interval(std::min(a[2], b[2]), std::max(a[2], b[2]));
    padToMinimums();
}

bounds3d::bounds3d(const bounds3d& a, const bounds3d& b)
    : x(a.x, b.x), y(a.y, b.y), z(a.z, b.z) {}

bounds3d operator+(const bounds3d& bbox, const vec3d& offset) {
    return {bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z()};
}

bounds3d operator+(const vec3d& offset, const bounds3d& bbox) {
    return bbox + offset;
}

const interval& bounds3d::axis_interval(int n) const {
    return (n == 0) ? x : (n == 1 ? y : z);
}

int bounds3d::longestAxis() const {
    if (x.size() > y.size()) return x.size() > z.size() ? 0 : 2;
    return y.size() > z.size() ? 1 : 2;
}

bool bounds3d::intersect(const ray& r, interval ray_t) const {
    for (int a = 0; a < 3; a++) {
        double invD = 1.0 / r.d()[a];
        double t0 = (axis_interval(a).min - r.o()[a]) * invD;
        double t1 = (axis_interval(a).max - r.o()[a]) * invD;
        if (invD < 0.0) std::swap(t0, t1);
        ray_t.min = std::max(t0, ray_t.min);
        ray_t.max = std::min(t1, ray_t.max);
        if (ray_t.max <= ray_t.min) return false;
    }
    return true;
}

void bounds3d::padToMinimums() {
    constexpr double delta = 0.0001;
    if (x.size() < delta) x = x.expand(delta);
    if (y.size() < delta) y = y.expand(delta);
    if (z.size() < delta) z = z.expand(delta);
}

/* ---------------- ONB ---------------- */

orthonormal_base::orthonormal_base(const vec3d& n) {
    axis[2] = unit_vector(n);
    vec3d a = (std::fabs(axis[2].x()) > 0.9) ? vec3d(0,1,0) : vec3d(1,0,0);
    axis[1] = unit_vector(cross(axis[2], a));
    axis[0] = cross(axis[2], axis[1]);
}

vec3d orthonormal_base::transform(const vec3d& v) const {
    return v[0]*axis[0] + v[1]*axis[1] + v[2]*axis[2];
}

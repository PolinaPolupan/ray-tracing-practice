#ifndef RAY_TRACING_IN_ONE_WEEK_MATH_H
#define RAY_TRACING_IN_ONE_WEEK_MATH_H

#include "rtweekend.h"

class ray;

class vec3d {
public:
    double e[3];

    vec3d();
    vec3d(double e0, double e1, double e2);

    [[nodiscard]] double x() const { return e[0]; }
    [[nodiscard]] double y() const { return e[1]; }
    [[nodiscard]] double z() const { return e[2]; }

    vec3d operator-() const;
    double operator[](const int i) const { return e[i]; }
    double& operator[](const int i) { return e[i]; }

    vec3d& operator+=(const vec3d& v);
    vec3d& operator*=(double t);
    vec3d& operator/=(double t);

    [[nodiscard]] double length() const;
    [[nodiscard]] double length_squared() const;
    [[nodiscard]] bool near_zero() const;
};

/* free functions */
vec3d operator+(const vec3d& u, const vec3d& v);
vec3d operator-(const vec3d& u, const vec3d& v);
vec3d operator*(double t, const vec3d& v);
vec3d operator*(const vec3d& v, double t);
vec3d operator*(const vec3d& u, const vec3d& v);
vec3d operator/(const vec3d& v, double t);

double dot(const vec3d& u, const vec3d& v);
vec3d cross(const vec3d& u, const vec3d& v);
vec3d unit_vector(const vec3d& v);
vec3d reflect(const vec3d& v, const vec3d& n);
vec3d refract(const vec3d& uv, const vec3d& n, double etai_over_etat);

std::ostream& operator<<(std::ostream& out, const vec3d& v);

using point3d = vec3d;
using color   = vec3d;

/* ---------------- interval ---------------- */

class interval {
public:
    double min, max;

    interval();
    interval(double min, double max);
    interval(const interval& a, const interval& b);

    [[nodiscard]] double size() const;
    [[nodiscard]] bool contains(double x) const;
    [[nodiscard]] bool surrounds(double x) const;
    [[nodiscard]] double clamp(double x) const;
    [[nodiscard]] interval expand(double delta) const;

    static const interval empty, universe;
};

interval operator+(const interval& ival, double displacement);
interval operator+(double displacement, const interval& ival);

/* ---------------- bounds3d ---------------- */

class bounds3d {
public:
    interval x, y, z;

    bounds3d();
    bounds3d(const interval& x, const interval& y, const interval& z);
    bounds3d(const point3d& a, const point3d& b);
    bounds3d(const bounds3d& box0, const bounds3d& box1);

    [[nodiscard]] const interval& axis_interval(int n) const;
    [[nodiscard]] int longestAxis() const;
    [[nodiscard]] bool intersect(const ray& r, interval ray_t) const;

    static const bounds3d empty, universe;

private:
    void padToMinimums();
};

bounds3d operator+(const bounds3d& bbox, const vec3d& offset);
bounds3d operator+(const vec3d& offset, const bounds3d& bbox);

/* ---------------- ONB ---------------- */

class orthonormal_base {
public:
    explicit orthonormal_base(const vec3d& n);

    [[nodiscard]] const vec3d& u() const { return axis[0]; }
    [[nodiscard]] const vec3d& v() const { return axis[1]; }
    [[nodiscard]] const vec3d& w() const { return axis[2]; }

    [[nodiscard]] vec3d transform(const vec3d& v) const;

private:
    vec3d axis[3];
};

#endif

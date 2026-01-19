#ifndef RAY_TRACING_IN_ONE_WEEK_MATH_H
#define RAY_TRACING_IN_ONE_WEEK_MATH_H

#include "rtweekend.h"

class ray;

struct point2 {
    double x, y;
};

class vec3 {
public:
    double e[3];

    vec3();
    vec3(double e0, double e1, double e2);

    [[nodiscard]] double x() const { return e[0]; }
    [[nodiscard]] double y() const { return e[1]; }
    [[nodiscard]] double z() const { return e[2]; }

    vec3 operator-() const;
    double operator[](const int i) const { return e[i]; }
    double& operator[](const int i) { return e[i]; }

    vec3& operator+=(const vec3& v);
    vec3& operator*=(double t);
    vec3& operator*=(const vec3& v);
    vec3& operator/=(double t);

    [[nodiscard]] double length() const;
    [[nodiscard]] double length_squared() const;
    [[nodiscard]] bool near_zero() const;
};

/* free functions */
vec3 operator+(const vec3& u, const vec3& v);
vec3 operator-(const vec3& u, const vec3& v);
vec3 operator*(double t, const vec3& v);
vec3 operator*(const vec3& v, double t);
vec3 operator*(const vec3& u, const vec3& v);
vec3 operator/(const vec3& v, double t);

double dot(const vec3& u, const vec3& v);
vec3 cross(const vec3& u, const vec3& v);
vec3 unit_vector(const vec3& v);
vec3 reflect(const vec3& v, const vec3& n);
vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat);

std::ostream& operator<<(std::ostream& out, const vec3& v);

using point3 = vec3;
using color   = vec3;

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

class bounds3 {
public:
    interval x, y, z;

    bounds3();
    bounds3(const interval& x, const interval& y, const interval& z);
    bounds3(const point3& a, const point3& b);
    bounds3(const bounds3& box0, const bounds3& box1);

    [[nodiscard]] const interval& axis_interval(int n) const;
    [[nodiscard]] int longestAxis() const;
    [[nodiscard]] bool intersect(const ray& r, interval ray_t) const;

    static const bounds3 empty, universe;

private:
    void padToMinimums();
};

bounds3 operator+(const bounds3& bbox, const vec3& offset);
bounds3 operator+(const vec3& offset, const bounds3& bbox);

/* ---------------- ONB ---------------- */

class orthonormal_base {
public:
    explicit orthonormal_base(const vec3& n);

    [[nodiscard]] const vec3& u() const { return axis[0]; }
    [[nodiscard]] const vec3& v() const { return axis[1]; }
    [[nodiscard]] const vec3& w() const { return axis[2]; }

    [[nodiscard]] vec3 transform(const vec3& v) const;

private:
    vec3 axis[3];
};

#endif

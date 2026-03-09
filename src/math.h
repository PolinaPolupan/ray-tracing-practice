#ifndef RAY_TRACING_IN_ONE_WEEK_MATH_H
#define RAY_TRACING_IN_ONE_WEEK_MATH_H

#include "constants.h"

class ray;

template <typename T>
struct point2 {
    T x, y;

    double operator[](const int i) const {
        return (i == 0) ? x : y;
    }

    double& operator[](const int i) {
        return (i == 0) ? x : y;
    }

    [[nodiscard]] double min() const { return std::min(x, y); }
    [[nodiscard]] double max() const { return std::max(x, y); }
};

template <typename T>
point2<T> min(const point2<T>& p1, const point2<T>& p2)
{
    return (p1.min() < p2.min()) ? p1 : p2;
}

template <typename T>
point2<T> max(const point2<T>& p1, const point2<T>& p2)
{
    return (p1.max() < p2.max()) ? p2 : p1;
}

using  point2d = point2<double>;

class vec3 {
public:
    double e[3];

    vec3() : e{0,0,0} {}
    vec3(const double e0) : e{e0,e0,e0} {}
    vec3(const double e0, const double e1, const double e2) : e{e0,e1,e2} {}

    [[nodiscard]] double x() const { return e[0]; }
    [[nodiscard]] double y() const { return e[1]; }
    [[nodiscard]] double z() const { return e[2]; }

    vec3 operator-() const;
    double operator[](const int i) const { return e[i]; }
    double& operator[](const int i) { return e[i]; }
    explicit operator bool() const { return e[0] || e[1] || e[2]; }

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

/* ---------------- bounds2d ---------------- */
class bounds2i_iterator;

template <typename T>
class bounds2
{
public:
    bounds2(const point2<T>& p1, const point2<T>& p2): p_min(min(p1, p2)), p_max(max(p1, p2)) {}

    [[nodiscard]] bool is_empty() const { return p_min.x >= p_max.x || p_min.y >= p_max.y; }
    [[nodiscard]] bounds2i_iterator begin() const;
    [[nodiscard]] bounds2i_iterator end() const;

    point2<T> p_min, p_max;
};

template <typename T>
bounds2<T> intersect(const bounds2<T> &b1, const bounds2<T> &b2)
{
    return {min(b1.p_max, b2.p_max), max(b1.p_min, b2.p_min)};
}

using bounds2i = bounds2<int>;
using point2i = point2<int>;

class bounds2i_iterator {
public:
    bounds2i_iterator(const bounds2i* bounds, const point2i p): bounds(bounds), p(p) {}

    point2i operator*() const { return p; }

    bounds2i_iterator& operator++() {
        p.x++;

        if (p.x >= bounds->p_max.x) {
            p.x = bounds->p_min.x;
            p.y++;
        }

        return *this;
    }

    bool operator==(const bounds2i_iterator& other) const {
        return p.x == other.p.x && p.y == other.p.y;
    }

    bool operator!=(const bounds2i_iterator& other) const {
        return !(*this == other);
    }

private:
    const bounds2i* bounds;
    point2i p;
};

template <>
inline bounds2i_iterator bounds2i::begin() const {
    return {this,p_min};
}

template <>
inline bounds2i_iterator bounds2i::end() const {
    return {this,{p_min.x, p_max.y}};
}

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

class frame {
public:
    explicit frame(const vec3& n);

    [[nodiscard]] const vec3& u() const { return axis[0]; }
    [[nodiscard]] const vec3& v() const { return axis[1]; }
    [[nodiscard]] const vec3& w() const { return axis[2]; }

    [[nodiscard]] vec3 transform(const vec3& v) const;

    [[nodiscard]] vec3 to_local(const vec3& v) const
    {
        return {dot(v, axis[0]), dot(v, axis[1]), dot(v, axis[2])};
    }

    [[nodiscard]] vec3 from_local(const vec3& v) const
    {
        return {v.x() * axis[0] + v.y() * axis[1] + v.z() * axis[2]};
    }

private:
    vec3 axis[3];
};

#endif

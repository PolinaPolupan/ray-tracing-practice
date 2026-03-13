#ifndef MATH_H
#define MATH_H

#include "constants.h"

class ray;

template <typename T>
struct point2
{
    T x, y;

    double operator[](const int i) const
    { return (i == 0) ? x : y; }

    double& operator[](const int i)
    { return (i == 0) ? x : y; }

    [[nodiscard]] double min() const { return std::min(x, y); }
    [[nodiscard]] double max() const { return std::max(x, y); }
};

template <typename T>
point2<T> min(const point2<T>& p1, const point2<T>& p2) {
    return (p1.min() < p2.min()) ? p1 : p2;
}

template <typename T>
point2<T> max(const point2<T>& p1, const point2<T>& p2) {
    return (p1.max() < p2.max()) ? p2 : p1;
}

using point2d = point2<double>;

class vec3 {
public:
    double e[3];

    vec3() : e{0,0,0} {}
    vec3(const double e0) : e{e0,e0,e0} {}
    vec3(const double e0, const double e1, const double e2) : e{e0,e1,e2} {}

    [[nodiscard]] double x() const { return e[0]; }
    [[nodiscard]] double y() const { return e[1]; }
    [[nodiscard]] double z() const { return e[2]; }

    vec3 operator-() const { return {-e[0], -e[1], -e[2]}; }
    double operator[](const int i) const { return e[i]; }
    double& operator[](const int i) { return e[i]; }
    explicit operator bool() const { return e[0] || e[1] || e[2]; }

    vec3& operator+=(const vec3& v)
    {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*=(const double t)
    {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator*=(const vec3& v)
    {
        e[0] *= v.e[0];
        e[1] *= v.e[1];
        e[2] *= v.e[2];
        return *this;
    }

    vec3& operator/=(const double t) { return *this *= 1 / t; }

    [[nodiscard]] double length() const {
        return std::sqrt(length_squared());
    }

    [[nodiscard]] double length_squared() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }

    [[nodiscard]] bool near_zero() const
    {
        constexpr double s = 1e-8;
        return std::fabs(e[0]) < s && std::fabs(e[1]) < s && std::fabs(e[2]) < s;
    }
};

/* free functions */

inline vec3 operator*(const vec3& u, const vec3& v)
{
    return {
        u.e[0] * v.e[0],
        u.e[1] * v.e[1],
        u.e[2] * v.e[2]
    };
}

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return {u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]};
}

inline vec3 operator+(const vec3& u, const double v) {
    return {u.e[0] + v, u.e[1] + v, u.e[2] + v};
}


inline vec3 operator-(const vec3& u, const vec3& v) {
    return {u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]};
}

inline vec3 operator*(const double t, const vec3& v) {
    return {t*v.e[0], t*v.e[1], t*v.e[2]};
}

inline vec3 operator*(const vec3& v, const double t) {
    return t * v;
}

inline vec3 operator/(const vec3& v, const double t) {
    return (1/t) * v;
}

vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat);

inline double dot(const vec3& u, const vec3& v) {
    return u.e[0]*v.e[0] + u.e[1]*v.e[1] + u.e[2]*v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return {
        u.e[1]*v.e[2] - u.e[2]*v.e[1],
        u.e[2]*v.e[0] - u.e[0]*v.e[2],
        u.e[0]*v.e[1] - u.e[1]*v.e[0]
    };
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2*dot(v,n)*n;
}

using point3 = vec3;
using color  = vec3;

/* ---------------- interval ---------------- */

class interval {
public:
    double min, max;

    interval(): min(+infinity), max(-infinity) {}
    interval(const double min, const double max) : min(min), max(max) {}
    interval(const interval& a, const interval& b) :
    min(std::min(a.min, b.min)),
    max(std::max(a.max, b.max)) {}

    [[nodiscard]] double size() const { return max - min; }
    [[nodiscard]] bool contains(double x) const { return min <= x && x <= max; }
    [[nodiscard]] bool surrounds(double x) const { return min < x && x < max; }
    [[nodiscard]] double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }
    [[nodiscard]] interval expand(double delta) const {
        const double p = delta * 0.5;
        return {min - p, max + p};
    }

    static const interval empty, universe;
};

inline interval operator+(const interval& i, const double d) {
    return {i.min + d, i.max + d};
}

inline interval operator+(const double d, const interval& i) {
    return i + d;
}

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

    bounds3() = default;
    bounds3(const interval& x_, const interval& y_, const interval& z_): x(x_), y(y_), z(z_) {
        padToMinimums();
    }
    bounds3(const point3& a, const point3& b) {
        x = interval(std::min(a[0], b[0]), std::max(a[0], b[0]));
        y = interval(std::min(a[1], b[1]), std::max(a[1], b[1]));
        z = interval(std::min(a[2], b[2]), std::max(a[2], b[2]));
        padToMinimums();
    }
    bounds3(const bounds3& a, const bounds3& b) : x(a.x, b.x), y(a.y, b.y), z(a.z, b.z) {}

    [[nodiscard]] const interval& axis_interval(int n) const {
        return (n == 0) ? x : (n == 1 ? y : z);
    }
    [[nodiscard]] int longestAxis() const {
        if (x.size() > y.size()) return x.size() > z.size() ? 0 : 2;
        return y.size() > z.size() ? 1 : 2;
    }
    [[nodiscard]] bool intersect(const ray& r, interval ray_t) const;

    static const bounds3 empty, universe;

private:
    void padToMinimums();
};

inline bounds3 operator+(const bounds3& bbox, const vec3& offset) {
    return {bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z()};
}

inline bounds3 operator+(const vec3& offset, const bounds3& bbox) {
    return bbox + offset;
}

class frame {
public:
    explicit frame(const vec3& n) {
        _axis[2] = unit_vector(n);
        vec3 a = (std::fabs(_axis[2].x()) > 0.9) ? vec3(0,1,0) : vec3(1,0,0);
        _axis[1] = unit_vector(cross(_axis[2], a));
        _axis[0] = cross(_axis[2], _axis[1]);
    }

    [[nodiscard]] const vec3& u() const { return _axis[0]; }
    [[nodiscard]] const vec3& v() const { return _axis[1]; }
    [[nodiscard]] const vec3& w() const { return _axis[2]; }

    [[nodiscard]] vec3 transform(const vec3& v) const {
        return v[0]*_axis[0] + v[1]*_axis[1] + v[2]*_axis[2];
    }

    [[nodiscard]] vec3 to_local(const vec3& v) const {
        return {dot(v, _axis[0]), dot(v, _axis[1]), dot(v, _axis[2])};
    }

    [[nodiscard]] vec3 from_local(const vec3& v) const {
        return {v.x() * _axis[0] + v.y() * _axis[1] + v.z() * _axis[2]};
    }

private:
    vec3 _axis[3];
};

#endif

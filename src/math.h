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

template <typename T>
class vec3 {
public:
    T e[3];

    vec3() : e{0,0,0} {}
    vec3(const T e0) : e{e0,e0,e0} {}
    vec3(const T e0, const T e1, const T e2) : e{e0,e1,e2} {}

    [[nodiscard]] T x() const { return e[0]; }
    [[nodiscard]] T y() const { return e[1]; }
    [[nodiscard]] T z() const { return e[2]; }

    vec3 operator-() const { return {-e[0], -e[1], -e[2]}; }
    T operator[](const int i) const { return e[i]; }
    T& operator[](const int i) { return e[i]; }
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
template <typename T>
vec3<T> operator*(const vec3<T>& u, const vec3<T>& v)
{
    return {
        u.e[0] * v.e[0],
        u.e[1] * v.e[1],
        u.e[2] * v.e[2]
    };
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const vec3<T>& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

template <typename T>
vec3<T> operator+(const vec3<T>& u, const vec3<T>& v) {
    return {u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]};
}

template <typename T>
vec3<T> operator+(const vec3<T>& u, const double v) {
    return {u.e[0] + v, u.e[1] + v, u.e[2] + v};
}
template <typename T>
vec3<T> operator-(const vec3<T>& u, const vec3<T>& v) {
    return {u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]};
}

template <typename T>
vec3<T> operator*(const double t, const vec3<T>& v) {
    return {t*v.e[0], t*v.e[1], t*v.e[2]};
}

template <typename T>
vec3<T> operator*(const vec3<T>& v, const double t) {
    return t * v;
}

template <typename T>
vec3<T> operator/(const vec3<T>& v, const double t) {
    return (1/t) * v;
}

template <typename T>
vec3<T> refract(const vec3<T>& uv, const vec3<T>& n, const double etai_over_etat) {
    const double cos_theta = std::fmin(dot(-uv, n), 1.0);
    const vec3<T> r_out_perp = etai_over_etat * (uv + cos_theta*n);
    const vec3<T> r_out_parallel =
        -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

template <typename T>
double dot(const vec3<T>& u, const vec3<T>& v) {
    return u.e[0]*v.e[0] + u.e[1]*v.e[1] + u.e[2]*v.e[2];
}

template <typename T>
vec3<T> cross(const vec3<T>& u, const vec3<T>& v) {
    return {
        u.e[1]*v.e[2] - u.e[2]*v.e[1],
        u.e[2]*v.e[0] - u.e[0]*v.e[2],
        u.e[0]*v.e[1] - u.e[1]*v.e[0]
    };
}

template <typename T>
vec3<T> unit_vector(const vec3<T>& v) {
    return v / v.length();
}

template <typename T>
vec3<T> reflect(const vec3<T>& v, const vec3<T>& n) {
    return v - 2*dot(v,n)*n;
}

template <typename T>
vec3<T> min(const vec3<T>& a, const vec3<T>& b) {
    return {
        std::min(a[0], b[0]),
        std::min(a[1], b[1]),
        std::min(a[2], b[2])
    };
}

template <typename T>
vec3<T> max(const vec3<T>& a, const vec3<T>& b) {
    return {
        std::max(a[0], b[0]),
        std::max(a[1], b[1]),
        std::max(a[2], b[2])
    };
}

using color  = vec3<double>;
using vec3d = vec3<double>;
template <typename T>
using point3 = vec3<T>;
using point3d = vec3<double>;

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

template <typename T>
class bounds3 {
public:
    point3<T> p_min, p_max;

    bounds3() = default;

    bounds3(const point3<T>& a, const point3<T>& b) {
        p_min = min(a, b);
        p_max = max(a, b);
        pad_to_minimums();
    }

    bounds3(const bounds3& a, const bounds3& b) {
        p_min = min(a.p_min, b.p_min);
        p_max = max(a.p_max, b.p_max);
        pad_to_minimums();
    }

    vec3<T> diagonal() const { return p_max - p_min; }

    [[nodiscard]] int longest_axis() const {
        vec3<T> d = diagonal();
        if (d[0] > d[1] && d[0] > d[2]) return 0;
        if (d[1] > d[2]) return 1;
        return 2;
    }

    [[nodiscard]] bool intersect(const ray& r, interval ray_t) const;

    static const bounds3 empty, universe;

private:
    void pad_to_minimums() {
        constexpr T delta = static_cast<T>(0.0001);
        for (int i = 0; i < 3; i++) {
            if (p_max[i] - p_min[i] < delta) {
                T padding = (delta - (p_max[i] - p_min[i])) / 2;
                p_min[i] -= padding;
                p_max[i] += padding;
            }
        }
    }
};

using bounds3d = bounds3<double>;

template <typename T>
bounds3<T> operator+(const bounds3<T>& bbox, const vec3<T>& offset) {
    return {bbox.p_min + offset, bbox.p_max + offset};
}

template <typename T>
bounds3<T> operator+(const vec3<T>& offset, const bounds3<T>& bbox) {
    return bbox + offset;
}

class frame {
public:
    explicit frame(const vec3d& n) {
        _axis[2] = unit_vector(n);
        vec3d a = (std::fabs(_axis[2].x()) > 0.9) ? vec3d(0,1,0) : vec3d(1,0,0);
        _axis[1] = unit_vector(cross(_axis[2], a));
        _axis[0] = cross(_axis[2], _axis[1]);
    }

    [[nodiscard]] const vec3d& u() const { return _axis[0]; }
    [[nodiscard]] const vec3d& v() const { return _axis[1]; }
    [[nodiscard]] const vec3d& w() const { return _axis[2]; }

    [[nodiscard]] vec3d transform(const vec3d& v) const {
        return v[0]*_axis[0] + v[1]*_axis[1] + v[2]*_axis[2];
    }

    [[nodiscard]] vec3d to_local(const vec3d& v) const {
        return {dot(v, _axis[0]), dot(v, _axis[1]), dot(v, _axis[2])};
    }

    [[nodiscard]] vec3d from_local(const vec3d& v) const {
        return {v.x() * _axis[0] + v.y() * _axis[1] + v.z() * _axis[2]};
    }

private:
    vec3d _axis[3];
};

#endif

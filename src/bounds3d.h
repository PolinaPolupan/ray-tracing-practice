#ifndef AABB_H
#define AABB_H
#include "interval.h"
#include "ray.h"
#include "Vec3.h"


class bounds3d {
public:
    interval x, y, z;

    bounds3d() = default; // The default AABB is empty, since intervals are empty by default.

    bounds3d(const interval& x, const interval& y, const interval& z)
      : x(x), y(y), z(z) {
        padToMinimums();
    }

    bounds3d(const Point3& a, const Point3& b) {
        // Treat the two points a and b as extrema for the bounding box, so we don't require a
        // particular minimum/maximum coordinate order.

        x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);

        padToMinimums();
    }

    bounds3d(const bounds3d& box0, const bounds3d& box1) {
        x = interval(box0.x, box1.x);
        y = interval(box0.y, box1.y);
        z = interval(box0.z, box1.z);
    }

    [[nodiscard]] const interval& axis_interval(const int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    [[nodiscard]] int longestAxis() const {
        // Returns the index of the longest axis of the bounding box.
        if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        return y.size() > z.size() ? 1 : 2;
    }

    [[nodiscard]] bool intersect(const ray& r, interval ray_t) const {
        const Point3& ray_orig = r.o();
        const Vec3&   ray_dir  = r.d();

        for (int axis = 0; axis < 3; axis++) {
            const interval& ax = axis_interval(axis);
            const double adinv = 1.0 / ray_dir[axis];

            const auto t0 = (ax.min - ray_orig[axis]) * adinv;
            const auto t1 = (ax.max - ray_orig[axis]) * adinv;

            if (t0 < t1) {
                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            } else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }

    static const bounds3d empty, universe;

private:
    void padToMinimums() {
        // Adjust the AABB so that no side is narrower than some delta, padding if necessary.
        constexpr double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

inline const bounds3d bounds3d::empty    = bounds3d(interval::empty,    interval::empty,    interval::empty);
inline const bounds3d bounds3d::universe = bounds3d(interval::universe, interval::universe, interval::universe);

inline bounds3d operator+(const bounds3d& bbox, const Vec3& offset) {
    return {bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z()};
}

inline bounds3d operator+(const Vec3& offset, const bounds3d& bbox) {
    return bbox + offset;
}

#endif
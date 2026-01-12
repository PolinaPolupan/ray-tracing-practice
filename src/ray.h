#ifndef RAY_H
#define RAY_H

#include "Vec3.h"

class ray {
public:
    ray() = default;

    ray(const Point3& o, const Vec3& d, const double t)
        : o_(o), d_(d), time_(t) {}

    ray(const Point3& o, const Vec3& d)
        : o_(o), d_(d) {}

    [[nodiscard]] Point3 o() const { return o_; }
    [[nodiscard]] Vec3 d() const { return d_; }
    [[nodiscard]] double time() const { return time_; }

    [[nodiscard]] Point3 at(const double t) const { return o_ + t*d_; }

private:
    Point3 o_;
    Vec3 d_;
    double time_{};
};


#endif
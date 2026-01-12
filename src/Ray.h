#ifndef RAY_H
#define RAY_H

#include "Vec3.h"

class Ray {
public:
    Ray() = default;

    Ray(const Point3& origin, const Vec3& direction, double time): orig(origin), dir(direction), tm(time) {}
    Ray(const Point3& origin, const Vec3& direction) : orig(origin), dir(direction), tm(0) {}

    [[nodiscard]] const Point3& origin() const  { return orig; }
    [[nodiscard]] const Vec3& direction() const { return dir; }
    [[nodiscard]] double time() const { return tm; }

    [[nodiscard]] Point3 at(const double t) const {
        return orig + t*dir;
    }

private:
    Point3 orig;
    Vec3 dir;
    double tm;
};

#endif
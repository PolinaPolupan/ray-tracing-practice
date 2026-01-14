#ifndef RAY_H
#define RAY_H

#include "math.h"

class vec3;

class ray {
public:
    ray() = default;

    ray(const point3d& o, const vec3& d, const double t)
        : o_(o), d_(d), time_(t) {}

    ray(const point3d& o, const vec3& d)
        : o_(o), d_(d) {}

    [[nodiscard]] point3d o() const { return o_; }
    [[nodiscard]] vec3 d() const { return d_; }
    [[nodiscard]] double time() const { return time_; }

    [[nodiscard]] point3d at(const double t) const { return o_ + t*d_; }

private:
    point3d o_{};
    vec3 d_{};
    double time_{};
};


#endif
#ifndef RAY_H
#define RAY_H

class ray {
public:
    ray() = default;

    ray(const point3& o, const vec3d& d, const double t)
        : o_(o), d_(d), time_(t) {}

    ray(const point3& o, const vec3d& d)
        : o_(o), d_(d) {}

    [[nodiscard]] point3 o() const { return o_; }
    [[nodiscard]] vec3d d() const { return d_; }
    [[nodiscard]] double time() const { return time_; }

    [[nodiscard]] point3 at(const double t) const { return o_ + t*d_; }

private:
    point3 o_{};
    vec3d d_{};
    double time_{};
};


#endif
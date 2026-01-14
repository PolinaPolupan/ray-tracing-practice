#ifndef HITTABLE_H
#define HITTABLE_H
#include "HitRecord.h"


class shape {
public:
    virtual ~shape() = default;

    virtual bool intersect(const ray& r, interval ray_t, HitRecord& rec) const = 0;

    [[nodiscard]] virtual bounds3 bounds() const = 0;

    [[nodiscard]] virtual double pdf(const point3d& origin, const vec3& direction) const {
        return 0.0;
    }

    [[nodiscard]] virtual vec3 random(const point3d& origin) const {
        return {1,0,0};
    }
};

#endif
#ifndef HITTABLE_H
#define HITTABLE_H
#include "AABB.h"
#include "HitRecord.h"
#include "Interval.h"
#include "ray.h"


class shape {
public:
    virtual ~shape() = default;

    virtual bool intersect(const ray& r, Interval ray_t, HitRecord& rec) const = 0;

    [[nodiscard]] virtual AABB bounds() const = 0;

    [[nodiscard]] virtual double pdf(const Point3& origin, const Vec3& direction) const {
        return 0.0;
    }

    [[nodiscard]] virtual Vec3 random(const Point3& origin) const {
        return {1,0,0};
    }
};

#endif
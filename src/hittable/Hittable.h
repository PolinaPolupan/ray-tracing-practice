#ifndef HITTABLE_H
#define HITTABLE_H
#include "AABB.h"
#include "HitRecord.h"
#include "Interval.h"
#include "Ray.h"


class Hittable {
public:
    virtual ~Hittable() = default;

    virtual bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const = 0;

    [[nodiscard]] virtual AABB boundingBox() const = 0;

    [[nodiscard]] virtual double pdfValue(const Point3& origin, const Vec3& direction) const {
        return 0.0;
    }

    [[nodiscard]] virtual Vec3 random(const Point3& origin) const {
        return {1,0,0};
    }
};

#endif
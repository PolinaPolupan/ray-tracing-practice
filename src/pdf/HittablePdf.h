//
// Created by polup on 06/01/2026.
//

#ifndef HITTABLEPDF_H
#define HITTABLEPDF_H
#include "hittable/HittableList.h"


class HittablePdf final : public PDF {
public:
    HittablePdf(const HittableList& objects, const Point3& origin)
      : objects(objects), origin(origin)
    {}

    [[nodiscard]] double value(const Vec3& direction) const override {
        return objects.pdf(origin, direction);
    }

    [[nodiscard]] Vec3 generate() const override {
        return objects.random(origin);
    }

private:
    const HittableList& objects;
    Point3 origin;

};



#endif //HITTABLEPDF_H

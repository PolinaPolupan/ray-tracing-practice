//
// Created by polup on 06/01/2026.
//

#ifndef HITTABLEPDF_H
#define HITTABLEPDF_H
#include "hittable/HittableList.h"


class HittablePdf final : public PDF {
public:
    HittablePdf(const HittableList& objects, const point3d& origin)
      : objects(objects), origin(origin)
    {}

    [[nodiscard]] double value(const vec3d& direction) const override {
        return objects.pdf(origin, direction);
    }

    [[nodiscard]] vec3d generate() const override {
        return objects.random(origin);
    }

private:
    const HittableList& objects;
    point3d origin;

};



#endif //HITTABLEPDF_H

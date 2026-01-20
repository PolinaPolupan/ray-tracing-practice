//
// Created by polup on 06/01/2026.
//

#ifndef HITTABLEPDF_H
#define HITTABLEPDF_H
#include "PDF.h"
#include "hittable/HittableList.h"


class HittablePdf final : public PDF {
public:
    HittablePdf(const HittableList& objects, const point3& origin)
      : objects(objects), origin(origin)
    {}

    [[nodiscard]] double value(const vec3& direction) const override {
        return objects.pdf(origin, direction);
    }

    [[nodiscard]] vec3 generate(const std::shared_ptr<sampler>& sampler) const override {
        return objects.random(origin, sampler);
    }

private:
    const HittableList& objects;
    point3 origin;

};



#endif //HITTABLEPDF_H

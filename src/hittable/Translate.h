//
// Created by polup on 03/01/2026.
//

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "shape.h"

class Translate final : public shape {
public:
    Translate(const shared_ptr<shape>& object, const vec3& offset)
     : object(object), offset(offset)
    {
        bbox = object->bounds() + offset;
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, const interval ray_t) const override {
        const ray offset_r(r.o() - offset, r.d(), r.time());

        auto result = object->intersect(offset_r, ray_t);
        if (!result) return {};

        result->p += offset;
        return result;
    }

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> object;
    vec3 offset;
    bounds3 bbox;
};

#endif //TRANSLATE_H

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

    bool intersect(const ray& r, const interval ray_t, shape_intersection& rec) const override {
        // Move the ray backwards by the offset
        ray offset_r(r.o() - offset, r.d(), r.time());

        // Determine whether an intersection exists along the offset ray (and if so, where)
        if (!object->intersect(offset_r, ray_t, rec))
            return false;

        // Move the intersection point forwards by the offset
        rec.p += offset;

        return true;
    }

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> object;
    vec3 offset;
    bounds3 bbox;
};

#endif //TRANSLATE_H

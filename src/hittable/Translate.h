//
// Created by polup on 03/01/2026.
//

#ifndef TRANSLATE_H
#define TRANSLATE_H

class Translate final : public Hittable {
public:
    Translate(const shared_ptr<Hittable>& object, const Vec3& offset)
     : object(object), offset(offset)
    {
        bbox = object->boundingBox() + offset;
    }

    bool hit(const Ray& r, const Interval ray_t, HitRecord& rec) const override {
        // Move the ray backwards by the offset
        Ray offset_r(r.origin() - offset, r.direction(), r.time());

        // Determine whether an intersection exists along the offset ray (and if so, where)
        if (!object->hit(offset_r, ray_t, rec))
            return false;

        // Move the intersection point forwards by the offset
        rec.p += offset;

        return true;
    }

    [[nodiscard]] AABB boundingBox() const override { return bbox; }

private:
    shared_ptr<Hittable> object;
    Vec3 offset;
    AABB bbox;
};

#endif //TRANSLATE_H

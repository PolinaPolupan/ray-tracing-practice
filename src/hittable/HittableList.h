//
// Created by polup on 03/01/2026.
//

#ifndef HITTABLELIST_H
#define HITTABLELIST_H

#include "sampling.h"
#include "shape.h"


class HittableList final : public shape {
public:
    std::vector<shared_ptr<shape>> objects;

    HittableList() = default;
    explicit HittableList(const shared_ptr<shape>& object) { add(object); }

    void clear() { objects.clear(); }

    void add(const shared_ptr<shape>& object) {
        objects.push_back(object);
        bbox = bounds3d(bbox, object->bounds());
    }

    bool intersect(const ray& r, const interval ray_t, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->intersect(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    [[nodiscard]] bounds3d bounds() const override { return bbox; }

    [[nodiscard]] double pdf(const point3d& origin, const vec3d& direction) const override {
        const auto weight = 1.0 / objects.size();
        auto sum = 0.0;

        for (const auto& object : objects)
            sum += weight * object->pdf(origin, direction);

        return sum;
    }

    [[nodiscard]] vec3d random(const point3d& origin) const override {
        const auto int_size = static_cast<int>(objects.size());
        return objects[random_int(0, int_size-1)]->random(origin);
    }

private:
    bounds3d bbox;
};


#endif //HITTABLELIST_H

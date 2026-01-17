//
// Created by polup on 03/01/2026.
//

#ifndef HITTABLELIST_H
#define HITTABLELIST_H

#include <optional>

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
        bbox = bounds3(bbox, object->bounds());
    }

    std::optional<shape_intersection> intersect(const ray& r, const interval ray_t, shape_intersection& rec) const override {
        shape_intersection temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->intersect(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        if (hit_anything)
        {
            return {rec};
        }
        return {};
    }

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

    [[nodiscard]] double pdf(const point3& origin, const vec3& direction) const override {
        const auto weight = 1.0 / objects.size();
        auto sum = 0.0;

        for (const auto& object : objects)
            sum += weight * object->pdf(origin, direction);

        return sum;
    }

    [[nodiscard]] vec3 random(const point3& origin, const std::shared_ptr<sampler>& sampler) const override {
        int idx = static_cast<int>(sampler->gen_1d() * objects.size());
        idx = std::min(idx, static_cast<int>(objects.size()) - 1);

        return objects[idx]->random(origin, sampler);
    }

private:
    bounds3 bbox;
};


#endif //HITTABLELIST_H

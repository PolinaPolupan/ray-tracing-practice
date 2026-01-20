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

    auto begin() { return objects.begin(); }
    auto end() { return objects.end(); }

    auto begin() const { return objects.begin(); }
    auto end() const { return objects.end(); }

    void clear() { objects.clear(); }

    void add(const shared_ptr<shape>& object) {
        objects.push_back(object);
        bbox = bounds3(bbox, object->bounds());
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, const interval ray_t) const override {
        std::optional<shape_intersection> closest_hit;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (const auto temp_hit = object->intersect(r, interval(ray_t.min, closest_so_far))) {
                closest_so_far = temp_hit->t;
                closest_hit = temp_hit;
            }
        }

        return closest_hit;
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

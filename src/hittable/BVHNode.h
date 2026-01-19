//
// Created by polup on 25/12/2025.
//

#ifndef BVHNODE_H
#define BVHNODE_H
#include <algorithm>

#include "shape.h"
#include "HittableList.h"


class BVHNode final : public shape {
public:
    explicit BVHNode(HittableList list) : BVHNode(list.objects, 0, list.objects.size()) {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    BVHNode(std::vector<shared_ptr<shape>>& objects, size_t start, size_t end) {
        // Build the bounding box of the span of source objects.
        bbox = bounds3::empty;
        for (size_t object_index=start; object_index < end; object_index++)
            bbox = bounds3(bbox, objects[object_index]->bounds());

        const int axis = bbox.longestAxis();

        const auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;

        const size_t object_span = end - start;

        if (object_span == 1) {
            left = right = objects[start];
        } else if (object_span == 2) {
            left = objects[start];
            right = objects[start+1];
        } else {
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

            auto mid = start + object_span/2;
            left = make_shared<BVHNode>(objects, start, mid);
            right = make_shared<BVHNode>(objects, mid, end);
        }
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, const interval ray_t) const override {
        if (!bbox.intersect(r, ray_t))
            return {};

        auto left_hit = left->intersect(r, ray_t);

        interval right_interval = ray_t;
        if (left_hit) right_interval.max = left_hit->t;

        auto right_hit = right->intersect(r, right_interval);

        if (right_hit) return right_hit;
        return left_hit;
    }

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> left;
    shared_ptr<shape> right;
    bounds3 bbox;

    static bool box_compare(const shared_ptr<shape>& a, const shared_ptr<shape> &b, const int axis_index) {
        const auto a_axis_interval = a->bounds().axis_interval(axis_index);
        const auto b_axis_interval = b->bounds().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b) {
        return box_compare(a, b, 2);
    }
};



#endif //BVHNODE_H

#ifndef AGGREGATES_H
#define AGGREGATES_H
#include <memory>

#include "shapes.h"

struct node
{
    bounds3                  bbox = bounds3::empty;
    std::shared_ptr<shape>   leaf;                  // non-null => leaf
    std::unique_ptr<node>    children[2];           // non-null => internal child

    [[nodiscard]] bool is_leaf() const { return static_cast<bool>(leaf); }
};

class accelerator
{
public:
    virtual ~accelerator() = default;

    [[nodiscard]] virtual std::optional<shape_intersection>
    intersect(const ray& r, interval ray_t) const = 0;

    [[nodiscard]] bounds3 bounds() const { return bbox_; }

protected:
    bounds3 bbox_ = bounds3::empty;
};

inline bool box_compare(const shared_ptr<shape>& a, const shared_ptr<shape> &b, const int axis_index)
{
    const auto a_axis_interval = a->bounds().axis_interval(axis_index);
    const auto b_axis_interval = b->bounds().axis_interval(axis_index);
    return a_axis_interval.min < b_axis_interval.min;
}

inline bool box_x_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b)
{ return box_compare(a, b, 0); }

inline bool box_y_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b)
{ return box_compare(a, b, 1); }

inline bool box_z_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b)
{ return box_compare(a, b, 2); }


class bvh final : public accelerator {
public:
    explicit bvh(std::vector<std::shared_ptr<shape>> objects) {
        root_ = build(objects, 0, objects.size());
        if (root_) bbox_ = root_->bbox;
    }

    [[nodiscard]] std::optional<shape_intersection>
    intersect(const ray& r, const interval ray_t) const override {
        return _intersect(root_.get(), r, ray_t);
    }

private:
    std::unique_ptr<node> root_;

    static std::unique_ptr<node> build(
        std::vector<std::shared_ptr<shape>>& objects,
        const size_t start,
        const size_t end
    ) {
        if (start >= end) return nullptr;

        auto n = std::make_unique<node>();

        for (size_t i = start; i < end; ++i)
            n->bbox = bounds3(n->bbox, objects[i]->bounds());

        const size_t span = end - start;

        if (span == 1) {
            n->leaf = objects[start];
            return n;
        }

        const int axis = n->bbox.longestAxis();
        auto cmp = [axis](const std::shared_ptr<shape>& a, const std::shared_ptr<shape>& b) {
            return box_compare(a, b, axis);
        };

        std::sort(objects.begin() + start, objects.begin() + end, cmp);

        const size_t mid = start + span / 2;
        n->children[0] = build(objects, start, mid);
        n->children[1] = build(objects, mid, end);

        return n;
    }

    static std::optional<shape_intersection> _intersect(
        const node* n,
        const ray& r,
        const interval ray_t
    ) {
        if (!n) return {};
        if (!n->bbox.intersect(r, ray_t)) return {};

        if (n->is_leaf())
            return n->leaf->intersect(r, ray_t);

        auto left_hit = _intersect(n->children[0].get(), r, ray_t);

        interval right_interval = ray_t;
        if (left_hit) right_interval.max = left_hit->t;

        auto right_hit = _intersect(n->children[1].get(), r, right_interval);
        return right_hit ? right_hit : left_hit;
    }
};


#endif // AGGREGATES_H
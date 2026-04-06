#ifndef AGGREGATES_H
#define AGGREGATES_H
#include <memory>

#include "shapes.h"

struct node
{
    bounds3d                 bbox = bounds3d::empty;
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

    [[nodiscard]] bounds3d bounds() const { return bbox_; }

protected:
    bounds3d bbox_ = bounds3d::empty;
};

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

    enum class split_mode { MIDDLE, SAH };
    inline static auto split_mode_ = split_mode::SAH;

    struct bvh_split_bucket {
        int count{};
        bounds3d bounds;
    };

private:
    std::unique_ptr<node> root_;

    static std::unique_ptr<node> build(
        std::vector<std::shared_ptr<shape>>& objects,
        const size_t start,
        const size_t end
    ) {
        if (start >= end) return nullptr;

        auto n = std::make_unique<node>();
        bounds3d bounds;

        for (size_t i = start; i < end; ++i)
            bounds = expand(bounds, objects[i]->bounds());

        const size_t span = end - start;

        if (span == 1 || bounds.surface_area() == 0) {
            n->leaf = objects[start];
            n->bbox = bounds;
            return n;
        }

        bounds3d centroid_bounds;
        for (size_t i = start; i < end; ++i)
            centroid_bounds = expand(centroid_bounds, objects[i]->bounds().centroid());

        const int dim = centroid_bounds.longest_axis();

        // degenerate dimensions
        if (centroid_bounds.p_max[dim] == centroid_bounds.p_min[dim]) {
            n->leaf = objects[start];
            n->bbox = bounds;
            return n;
        }

        size_t mid = start + span / 2;

        switch (split_mode_) {
            case split_mode::MIDDLE: {
                middle_split(objects, start, end, dim);
                break;
            }
            case split_mode::SAH: {
                if (span == 2) {
                    middle_split(objects, start, end, dim);
                }

                constexpr int n_buckets = 12;
                bvh_split_bucket buckets[n_buckets];

                for (size_t i = start; i < end; ++i) {
                    int b = static_cast<int>(n_buckets * centroid_bounds.offset(objects[i]->bounds().centroid())[dim]);
                    b = std::min(b, n_buckets - 1);
                    b = std::max(b, 0);
                    buckets[b].count++;
                    buckets[b].bounds = expand(buckets[b].bounds, objects[i]->bounds());
                }

                constexpr int n_splits = n_buckets - 1;
                double costs[n_splits] = {};

                int count_left = 0;
                bounds3d bounds_left;
                for (int i = 0; i < n_splits; ++i) {
                    bounds_left = expand(bounds_left, buckets[i].bounds);
                    count_left += buckets[i].count;
                    costs[i] += count_left * bounds_left.surface_area();
                }

                int count_right = 0;
                bounds3d bounds_right;
                for (int i = n_splits; i >= 1; --i) {
                    bounds_right = expand(bounds_right, buckets[i].bounds);
                    count_right += buckets[i].count;
                    costs[i - 1] += count_right * bounds_right.surface_area();
                }

                int min_cost_split_bucket = -1;
                double min_cost = infinity;

                for (int i = 0; i < n_splits; ++i) {
                    if (costs[i] < min_cost) {
                        min_cost = costs[i];
                        min_cost_split_bucket = i;
                    }
                }

                const size_t leaf_cost = span;
                constexpr size_t max_prims_in_node = 4;

                bool should_split = (span > max_prims_in_node) && (min_cost < leaf_cost);

                if (!should_split) {
                    // fallback to middle split instead of collapsing
                    middle_split(objects, start, end, dim);
                    mid = start + span / 2;
                } else {
                    auto midIter = std::partition(
                        objects.begin() + start, objects.begin() + end,
                        [=](const std::shared_ptr<shape>& obj) {
                            int b = static_cast<int>(
                                n_buckets * centroid_bounds.offset(obj->bounds().centroid())[dim]);
                            b = std::min(b, n_buckets - 1);
                            return b <= min_cost_split_bucket;
                        });
                    mid = midIter - objects.begin();
                }

                if (mid == start || mid == end) {
                    mid = start + span / 2;
                }
            }
        }

        n->children[0] = build(objects, start, mid);
        n->children[1] = build(objects, mid, end);
        n->bbox = bounds;

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

    static void middle_split(
        std::vector<std::shared_ptr<shape>>& objects,
        const size_t start,
        const size_t end,
        const int dim)
    {
        const size_t mid = start + (end - start) / 2;

        auto cmp = [dim](const std::shared_ptr<shape>& a, const std::shared_ptr<shape>& b) {
            return a->bounds().centroid()[dim] < b->bounds().centroid()[dim];
        };

        std::nth_element(objects.begin() + start,
                         objects.begin() + mid,
                         objects.begin() + end,
                         cmp);
    }
};

#endif // AGGREGATES_H
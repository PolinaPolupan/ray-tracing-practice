#include "accel.h"

std::optional<shape_intersection> Bvh::intersect(const ray& r, interval ray_t) const
{
    std::optional<shape_intersection> si;

    vec3d inv_dir(1 / r.d().x(), 1 / r.d().y(), 1 / r.d().z());
    const int dir_is_neg[3] = {
        inv_dir.x() < 0,
        inv_dir.y() < 0,
        inv_dir.z() < 0
    };

    if (nodes_.empty()) {
        return si;
    }

    int to_visit_offset = 0;
    int current_node_index = 0;
    int nodes_to_visit[64];

    while (true) {
        const linear_node* node = &nodes_[current_node_index];

        if (node->bounds_.intersect(r, ray_t)) {
            if (node->n_primitives_ > 0) {
                for (int i = 0; i < node->n_primitives_; ++i) {
                    auto prim_si =
                        ordered_prims_[node->offset_ + i]->intersect(r, ray_t);

                    if (prim_si) {
                        si = prim_si;
                        ray_t.max = si->t;
                    }
                }
                if (to_visit_offset == 0) break;
                current_node_index = nodes_to_visit[--to_visit_offset];
            } else {
                if (dir_is_neg[node->dim_]) {
                    nodes_to_visit[to_visit_offset++] = current_node_index + 1;
                    current_node_index = node->second_child_offset_;
                } else {
                    nodes_to_visit[to_visit_offset++] = node->second_child_offset_;
                    current_node_index = current_node_index + 1;
                }
            }
        } else {
            if (to_visit_offset == 0) break;
            current_node_index = nodes_to_visit[--to_visit_offset];
        }
    }

    return si;
}

std::unique_ptr<node> Bvh::build(
    std::vector<std::shared_ptr<shape>>& objects,
    std::vector<shape*>& ordered_prims,
    const size_t start,
    const size_t end,
    int& offset,
    int& total_nodes
    )
{
    total_nodes++;

     if (start >= end) {
         return nullptr;
     }

     auto n = std::make_unique<node>();
     bounds3d bounds;

     for (size_t i = start; i < end; ++i) {
         bounds = expand(bounds, objects[i]->bounds());
     }

     const size_t span = end - start;

     if (span == 1 || bounds.surface_area() == 0) {
         const int first_prim_offset = offset;

         for (size_t i = start; i < end; ++i) {
             ordered_prims[first_prim_offset + (i - start)] = objects[i].get();
         }

         offset += span;
         n->init_leaf(first_prim_offset, span, bounds);
         return n;
     }

     bounds3d centroid_bounds;

     for (size_t i = start; i < end; ++i) {
         centroid_bounds = expand(
             centroid_bounds,
             objects[i]->bounds().centroid());
     }

     const int dim = centroid_bounds.longest_axis();

     if (centroid_bounds.p_max[dim] == centroid_bounds.p_min[dim]) {
         const int first_prim_offset = offset;

         for (size_t i = start; i < end; ++i) {
             ordered_prims[first_prim_offset + (i - start)] = objects[i].get();
         }

         offset += span;
         n->init_leaf(first_prim_offset, span, bounds);
         return n;
     }

     size_t mid = start + span / 2;

     switch (split_mode) {
         case split_mode::middle: {
             middle_split(objects, start, end, dim);
             break;
         }

         case split_mode::sah: {
             if (span == 2) {
                 middle_split(objects, start, end, dim);
                 break;
             }

             constexpr int n_buckets = 12;
             bvh_split_bucket buckets[n_buckets];

             for (size_t i = start; i < end; ++i) {
                 int b = static_cast<int>(
                     n_buckets *
                     centroid_bounds.offset(objects[i]->bounds().centroid())[dim]);

                 b = std::clamp(b, 0, n_buckets - 1);

                 buckets[b].count_++;
                 buckets[b].bounds_ =
                     expand(buckets[b].bounds_, objects[i]->bounds());
             }

             constexpr int n_splits = n_buckets - 1;
             double costs[n_splits] = {};

             int count_left = 0;
             bounds3d bounds_left;

             for (int i = 0; i < n_splits; ++i) {
                 bounds_left = expand(bounds_left, buckets[i].bounds_);
                 count_left += buckets[i].count_;
                 costs[i] += count_left * bounds_left.surface_area();
             }

             int count_right = 0;
             bounds3d bounds_right;

             for (int i = n_splits; i >= 1; --i) {
                 bounds_right = expand(bounds_right, buckets[i].bounds_);
                 count_right += buckets[i].count_;
                 costs[i - 1] += count_right * bounds_right.surface_area();
             }

             int min_split = -1;
             double min_cost = infinity;

             for (int i = 0; i < n_splits; ++i) {
                 if (costs[i] < min_cost) {
                     min_cost = costs[i];
                     min_split = i;
                 }
             }

             const size_t leaf_cost = span;
             constexpr size_t max_prims_in_node = 4;

             min_cost = 0.5 + (min_cost / bounds.surface_area());

             const bool should_split =
                 (span > max_prims_in_node) || (min_cost < leaf_cost);

             if (!should_split) {
                 const int first_prim_offset = offset;

                 for (size_t i = start; i < end; ++i) {
                     ordered_prims[first_prim_offset + (i - start)] =
                         objects[i].get();
                 }

                 offset += span;
                 n->init_leaf(first_prim_offset, span, bounds);
                 return n;
             }

             auto mid_iter = std::partition(
                 objects.begin() + start,
                 objects.begin() + end,
                 [=](const std::shared_ptr<shape>& obj) {
                     int b = static_cast<int>(
                         n_buckets *
                         centroid_bounds.offset(obj->bounds().centroid())[dim]);

                     b = std::min(b, n_buckets - 1);
                     return b <= min_split;
                 });

             mid = mid_iter - objects.begin();
             break;
         }
     }

     n->init_interior(
         build(objects, ordered_prims, start, mid, offset, total_nodes),
         build(objects, ordered_prims, mid, end, offset, total_nodes),
         dim);

     return n;
}

int Bvh::flatten(const std::unique_ptr<node>& node, int& offset) {
    linear_node& ln = nodes_[offset];
    ln.bounds_ = node->bounds_;

    const int node_offset = offset++;

    if (node->n_primitives_ > 0) {
        ln.offset_ = node->offset_;
        ln.n_primitives_ = node->n_primitives_;
    } else {
        ln.n_primitives_ = 0;
        ln.dim_ = node->dim_;

        flatten(node->children_[0], offset);
        ln.second_child_offset_ = flatten(node->children_[1], offset);
    }

    return node_offset;
}

void Bvh::middle_split(
    std::vector<std::shared_ptr<shape>>& objects,
    const size_t start,
    const size_t end,
    int dim)
{
     const size_t mid = start + (end - start) / 2;

     auto cmp = [dim](const std::shared_ptr<shape>& a,
                      const std::shared_ptr<shape>& b) {
         return a->bounds().centroid()[dim] <
                b->bounds().centroid()[dim];
     };

     std::nth_element(
         objects.begin() + start,
         objects.begin() + mid,
         objects.begin() + end,
         cmp);
}

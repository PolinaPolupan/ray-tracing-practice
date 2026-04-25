#ifndef AGGREGATES_H
#define AGGREGATES_H

#include <memory>
#include <vector>
#include <optional>
#include <algorithm>

#include "shapes.h"

struct node {
    bounds3d bounds_ = bounds3d::empty;
    std::unique_ptr<node> children_[2];

    int offset_ = 0;
    int n_primitives_ = 0;
    int dim_ = 0;

    void init_leaf(const int offset, const int n, const bounds3d& bounds) {
        offset_ = offset;
        n_primitives_ = n;
        bounds_ = bounds;
    }

    void init_interior(std::unique_ptr<node> a,
                       std::unique_ptr<node> b,
                       const int dim) {
        children_[0] = std::move(a);
        children_[1] = std::move(b);

        n_primitives_ = 0;
        bounds_ = expand(children_[0]->bounds_, children_[1]->bounds_);
        dim_ = dim;
    }
};

struct linear_node {
    bounds3d bounds_ = bounds3d::empty;
    int offset_ = 0;
    int second_child_offset_ = 0;
    int n_primitives_ = 0;
    int dim_ = 0;
};

class Accelerator {
public:
    virtual ~Accelerator() = default;

    [[nodiscard]] virtual auto
    intersect(const ray& r, interval ray_t) const -> std::optional<shape_intersection> = 0;

    [[nodiscard]] auto bounds() const -> bounds3d { return bbox_; }

protected:
    bounds3d bbox_ = bounds3d::empty;
};

/** Bounding Volume Hierarchy (BVH) acceleration structure.
 *
 * Stores objects in a binary tree, then flattens it into a linear array
 * for cache-friendly ray traversal.
 *
 * Example scene with 4 primitives:
 *   Primitives A,B,C,D with centroids at x={1,2,6,7}
 *
 * Build produces tree:
 *          Root
 *         /    \
 *      Node1   Node2
 *     /   \   /   \
 *    A     B C     D
 *
 * Flattened linear nodes:
 * Index | Node Type | Data
 * 0     | Interior  | axis=x, second_child_offset=4
 * 1     | Interior  | axis=x, second_child_offset=3
 * 2     | Leaf      | offset=0 (A)
 * 3     | Leaf      | offset=1 (B)
 * 4     | Interior  | axis=x, second_child_offset=6
 * 5     | Leaf      | offset=2 (C)
 * 6     | Leaf      | offset=3 (D)
 */
class Bvh final : public Accelerator {
public:
    /** Construct BVH from objects. Builds tree and flattens it.
     * Example:
     *   std::vector<std::shared_ptr<shape>> objs = {A,B,C,D};
     *   Bvh bvh(objs);
     */
    explicit Bvh(std::vector<std::shared_ptr<shape>> objects) {
        int offset = 0;
        ordered_prims_.resize(objects.size());

        int total_nodes = 0;
        root_ = build(objects, ordered_prims_, 0, objects.size(), offset, total_nodes);

        if (root_) {
            bbox_ = root_->bounds_;
        }

        nodes_.resize(total_nodes);

        int linear_offset = 0;
        flatten(root_, linear_offset);
    }

    /** Traverse BVH and return first intersection of ray.
     *
     * Traverses linear nodes in depth-first order using a small stack.
     * Example: ray hits primitive B at t=1.2:
     *   intersect(ray, {0, infinity}) -> {B, 1.2}
     */
    [[nodiscard]] auto
    intersect(const ray& r, interval ray_t) const -> std::optional<shape_intersection> override;

    enum class split_mode { middle, sah };
    inline static split_mode split_mode = split_mode::sah;

    struct bvh_split_bucket {
        int count_ = 0;
        bounds3d bounds_;
    };

private:
    /** Recursive BVH builder.
     * Returns root node of subtree.
     *
     * Example:
     *   Primitives A,B,C,D with centroids x={1,2,6,7}
     *   build(...) produces tree:
     *          Root
     *         /    \
     *      Node1   Node2
     *     /   \   /   \
     *    A     B C     D
     */
    static auto build(
        std::vector<std::shared_ptr<shape>>& objects,
        std::vector<shape*>& ordered_prims,
        size_t start,
        size_t end,
        int& offset,
        int& total_nodes
    ) -> std::unique_ptr<node>;

    /** Flatten tree into linear array for fast traversal.
     *
     * Example tree:
     *       Root
     *      /    \
     *   Node1   Node2
     *  /   \   /   \
     * A     B C     D
     *
     * Becomes linear array:
     * 0=Root, 1=Node1, 2=A, 3=B, 4=Node2, 5=C, 6=D
     */
    auto flatten(const std::unique_ptr<node>& node, int& offset) -> int;

    /** Middle-split helper: split objects by median along axis. */
    static void middle_split(
        std::vector<std::shared_ptr<shape>>& objects,
        size_t start,
        size_t end,
        int dim);


    std::unique_ptr<node> root_;
    std::vector<shape*> ordered_prims_;
    std::vector<linear_node> nodes_;
};

#endif // AGGREGATES_H
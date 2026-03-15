#include "math.h"

#include "ray.h"

template <typename T>
bool bounds3<T>::intersect(const ray& r, interval ray_t) const {
    for (int a = 0; a < 3; a++) {
        const double invD = 1.0 / r.d()[a];
        double t0 = (p_min[a] - r.o()[a]) * invD;
        double t1 = (p_max[a] - r.o()[a]) * invD;
        if (invD < 0.0) std::swap(t0, t1);
        ray_t.min = std::max(t0, ray_t.min);
        ray_t.max = std::min(t1, ray_t.max);
        if (ray_t.max <= ray_t.min) return false;
    }
    return true;
}

template class bounds3<int>;
template class bounds3<double>;

template <> const bounds3<int> bounds3<int>::empty = bounds3<int>();
template <> const bounds3<int> bounds3<int>::universe = bounds3<int>();
template <> const bounds3<double> bounds3<double>::empty = bounds3<double>();
template <> const bounds3<double> bounds3<double>::universe = bounds3<double>();

const interval interval::empty    = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);

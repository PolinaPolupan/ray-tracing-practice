#ifndef INTERVAL_H
#define INTERVAL_H
#include "rtweekend.h"

class Interval {
public:
    double min, max;

    Interval() : min(+infinity), max(-infinity) {} // Default interval is empty

    Interval(const double min, const double max) : min(min), max(max) {}

    Interval(const Interval& a, const Interval& b) {
        // Create the interval tightly enclosing the two input intervals.
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }

    [[nodiscard]] double size() const {
        return max - min;
    }

    [[nodiscard]] bool contains(const double x) const {
        return min <= x && x <= max;
    }

    [[nodiscard]] bool surrounds(const double x) const {
        return min < x && x < max;
    }

    [[nodiscard]] double clamp(const double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    [[nodiscard]] Interval expand(double delta) const {
        const auto padding = delta/2;
        return {min - padding, max + padding};
    }

    static const Interval empty, universe;
};

inline const Interval Interval::empty    = Interval(+infinity, -infinity);
inline const Interval Interval::universe = Interval(-infinity, +infinity);

inline Interval operator+(const Interval& ival, const double displacement) {
    return {ival.min + displacement, ival.max + displacement};
}

inline Interval operator+(const double displacement, const Interval& ival) {
    return ival + displacement;
}

#endif
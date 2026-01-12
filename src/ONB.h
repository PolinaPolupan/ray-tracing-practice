//
// Created by polup on 06/01/2026.
//

#ifndef ONB_H
#define ONB_H
#include "Vec3.h"

class ONB {
public:
    explicit ONB(const Vec3& n) {
        axis[2] = unit_vector(n);
        const Vec3 a = (std::fabs(axis[2].x()) > 0.9) ? Vec3(0,1,0) : Vec3(1,0,0);
        axis[1] = unit_vector(cross(axis[2], a));
        axis[0] = cross(axis[2], axis[1]);
    }

    [[nodiscard]] const Vec3& u() const { return axis[0]; }
    [[nodiscard]] const Vec3& v() const { return axis[1]; }
    [[nodiscard]] const Vec3& w() const { return axis[2]; }

    [[nodiscard]] Vec3 transform(const Vec3& v) const {
        // Transform from basis coordinates to local space.
        return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
    }

private:
    Vec3 axis[3];
};

#endif //ONB_H

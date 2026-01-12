//
// Created by polup on 03/01/2026.
//

#ifndef HITRECORD_H
#define HITRECORD_H
#include "ray.h"
#include "Vec3.h"


class Material;

class HitRecord {
public:
    Point3 p;
    Vec3 normal;
    shared_ptr<Material> mat;
    double t{};
    double u;
    double v;
    bool front_face{};

    void set_face_normal(const ray& r, const Vec3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.d(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

#endif //HITRECORD_H

//
// Created by polup on 03/01/2026.
//

#ifndef CONSTANTMEDIUM_H
#define CONSTANTMEDIUM_H
#include "shape.h"
#include "material/Isotropic.h"


class Texture;

class ConstantMedium final : public shape {
public:
    ConstantMedium(const shared_ptr<shape> &boundary, const double density, const shared_ptr<Texture>& tex)
      : boundary(boundary), neg_inv_density(-1/density),
        phase_function(make_shared<Isotropic>(tex))
    {}

    ConstantMedium(const shared_ptr<shape> &boundary, const double density, const Color& albedo)
      : boundary(boundary), neg_inv_density(-1/density),
        phase_function(make_shared<Isotropic>(albedo))
    {}

    bool intersect(const ray& r, const interval ray_t, HitRecord& rec) const override {
        HitRecord rec1, rec2;

        if (!boundary->intersect(r, interval::universe, rec1))
            return false;

        if (!boundary->intersect(r, interval(rec1.t+0.0001, infinity), rec2))
            return false;

        if (rec1.t < ray_t.min) rec1.t = ray_t.min;
        if (rec2.t > ray_t.max) rec2.t = ray_t.max;

        if (rec1.t >= rec2.t)
            return false;

        if (rec1.t < 0)
            rec1.t = 0;

        const auto ray_length = r.direction().length();
        const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
        const auto hit_distance = neg_inv_density * std::log(random_double());

        if (hit_distance > distance_inside_boundary)
            return false;

        rec.t = rec1.t + hit_distance / ray_length;
        rec.p = r.at(rec.t);

        rec.normal = Vec3(1,0,0);  // arbitrary
        rec.front_face = true;     // also arbitrary
        rec.mat = phase_function;

        return true;
    }

    [[nodiscard]] bounds3d bounds() const override { return boundary->bounds(); }

private:
    shared_ptr<shape> boundary;
    double neg_inv_density;
    shared_ptr<Material> phase_function;
};

#endif //CONSTANTMEDIUM_H

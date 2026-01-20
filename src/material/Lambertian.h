//
// Created by polup on 04/11/2025.
//

#ifndef LAMBERTIAN_H
#define LAMBERTIAN_H
#include "Material.h"

class Texture;

class Lambertian final : public Material {
public:
    explicit Lambertian(const color& albedo) : tex(make_shared<SolidColor>(albedo)) {}
    explicit Lambertian(const shared_ptr<Texture> &tex) : tex(tex) {}

    std::unique_ptr<bsdf> get_bsdf(const shape_intersection& rec) const override {
        color albedo = tex->value(rec.u, rec.v, rec.p);
        return std::make_unique<lambertian_bsdf>(albedo, rec.normal);
    }

private:
    shared_ptr<Texture> tex;
};

#endif //LAMBERTIAN_H

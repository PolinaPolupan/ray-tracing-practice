//
// Created by polup on 02/01/2026.
//

#ifndef DIFFUSELIGHT_H
#define DIFFUSELIGHT_H
#include <memory>

#include "texture/SolidColor.h"

class Texture;

class DiffuseLight final : public Material {
public:
    explicit DiffuseLight(const std::shared_ptr<Texture>& tex) : tex(tex) {}
    explicit DiffuseLight(const color& emit)
        : tex(make_shared<SolidColor>(emit)) {}

    std::unique_ptr<bsdf> get_bsdf(const shape_intersection&) const override {
        return nullptr;
    }

    color Le(const ray&, const shape_intersection& rec, double u, double v, const point3& p) const override {
        if (!rec.front_face)
            return color(0,0,0);
        return tex->value(u, v, p);
    }

private:
    shared_ptr<Texture> tex;
};


#endif //DIFFUSELIGHT_H

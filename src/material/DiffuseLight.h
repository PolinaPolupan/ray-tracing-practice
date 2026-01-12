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
    explicit DiffuseLight(const std::shared_ptr<Texture> &tex) : tex(tex) {}
    explicit DiffuseLight(const Color& emit) : tex(make_shared<SolidColor>(emit)) {}

    [[nodiscard]] Color emitted(const ray& r_in, const HitRecord& rec, const double u, const double v, const Point3& p) const override {
        if (!rec.front_face)
            return {0,0,0};
        return tex->value(u, v, p);
    }

private:
    shared_ptr<Texture> tex;
};

#endif //DIFFUSELIGHT_H

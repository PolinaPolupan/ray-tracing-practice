//
// Created by polup on 01/01/2026.
//

#ifndef NOISETEXTURE_H
#define NOISETEXTURE_H
#include "Color.h"
#include "Perlin.h"
#include "Texture.h"


class NoiseTexture final : public Texture {
public:
    explicit NoiseTexture(const double scale) : scale(scale) {}

    [[nodiscard]] Color value(double u, double v, const Point3& p) const override {
        return Color(.5, .5, .5) * (1 + std::sin(scale * p.z() + 10 * noise.turb(p, 7)));
    }

private:
    Perlin noise{};
    double scale;
};

#endif //NOISETEXTURE_H

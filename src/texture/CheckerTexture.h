//
// Created by polup on 25/12/2025.
//

#ifndef CHECKERTEXTURE_H
#define CHECKERTEXTURE_H
#include <memory>

#include "math.h"
#include "SolidColor.h"
#include "Texture.h"


class CheckerTexture final : public Texture {
public:
    CheckerTexture(const double scale, const std::shared_ptr<Texture> &even, const std::shared_ptr<Texture> &odd)
      : inv_scale(1.0 / scale), even(even), odd(odd) {}

    CheckerTexture(const double scale, const color& c1, const color& c2)
      : CheckerTexture(scale, std::make_shared<SolidColor>(c1), std::make_shared<SolidColor>(c2)) {}

    [[nodiscard]] color value(const double u, const double v, const point3d& p) const override {
        const auto xInteger = static_cast<int>(std::floor(inv_scale * p.x()));
        const auto yInteger = static_cast<int>(std::floor(inv_scale * p.y()));
        const auto zInteger = static_cast<int>(std::floor(inv_scale * p.z()));

        const bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    double inv_scale;
    shared_ptr<Texture> even;
    shared_ptr<Texture> odd;
};



#endif //CHECKERTEXTURE_H

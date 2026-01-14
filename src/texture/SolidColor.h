//
// Created by polup on 25/12/2025.
//

#ifndef SOLIDCOLOR_H
#define SOLIDCOLOR_H
#include "Texture.h"

class SolidColor final : public Texture {
public:
    explicit SolidColor(const color& albedo) : albedo(albedo) {}

    SolidColor(const double red, const double green, const double blue) : SolidColor(color(red,green,blue)) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override {
        return albedo;
    }

private:
    color albedo;
};

#endif //SOLIDCOLOR_H

//
// Created by polup on 25/12/2025.
//

#ifndef SOLIDCOLOR_H
#define SOLIDCOLOR_H
#include "Texture.h"

class SolidColor final : public Texture {
public:
    explicit SolidColor(const Color& albedo) : albedo(albedo) {}

    SolidColor(const double red, const double green, const double blue) : SolidColor(Color(red,green,blue)) {}

    [[nodiscard]] Color value(double u, double v, const Point3& p) const override {
        return albedo;
    }

private:
    Color albedo;
};

#endif //SOLIDCOLOR_H

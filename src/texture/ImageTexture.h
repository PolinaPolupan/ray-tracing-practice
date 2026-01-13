//
// Created by polup on 01/01/2026.
//

#ifndef IMAGETEXTURE_H
#define IMAGETEXTURE_H
#include "RtwImage.h"


class ImageTexture final : public Texture {
public:
    explicit ImageTexture(const char* filename) : image(filename) {}

    [[nodiscard]] Color value(double u, double v, const Point3& p) const override {
        // If we have no texture data, then return solid cyan as a debugging aid.
        if (image.height() <= 0) return {0,1,1};

        // Clamp input texture coordinates to [0,1] x [1,0]
        u = interval(0,1).clamp(u);
        v = 1.0 - interval(0,1).clamp(v);  // Flip V to image coordinates

        const auto i = static_cast<int>(u * image.width());
        const auto j = static_cast<int>(v * image.height());
        const auto pixel = image.pixelData(i,j);

        constexpr auto colorScale = 1.0 / 255.0;
        return {colorScale*pixel[0], colorScale*pixel[1], colorScale*pixel[2]};
    }

private:
    RtwImage image;
};

#endif //IMAGETEXTURE_H

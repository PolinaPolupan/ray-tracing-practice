#ifndef RAY_TRACING_IN_ONE_WEEK_TEXTURES_H
#define RAY_TRACING_IN_ONE_WEEK_TEXTURES_H
#include "math.h"
#include "RtwImage.h"


class texture {
public:
    virtual ~texture() = default;

    [[nodiscard]] virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color final : public texture {
public:
    explicit solid_color(const color& albedo) : albedo_(albedo) {}

    solid_color(const double red, const double green, const double blue) : solid_color(color(red,green,blue)) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override {
        return albedo_;
    }

private:
    color albedo_;
};

class noise final : public texture {
public:
    explicit noise(const double scale) : scale_(scale) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override {
        return color(.5, .5, .5) * (1 + std::sin(scale_ * p.z() + 10 * noise.turb(p, 7)));
    }

private:
    Perlin noise{};
    double scale_;
};

class image final : public texture {
public:
    explicit image(const char* filename) : image(filename) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override {
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

class checker final : public texture {
public:
    checker(const double scale, const std::shared_ptr<texture> &even, const std::shared_ptr<texture> &odd)
      : inv_scale_(1.0 / scale), even_(even), odd_(odd) {}

    checker(const double scale, const color& c1, const color& c2)
      : checker(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2)) {}

    [[nodiscard]] color value(const double u, const double v, const point3& p) const override {
        const auto xInteger = static_cast<int>(std::floor(inv_scale_ * p.x()));
        const auto yInteger = static_cast<int>(std::floor(inv_scale_ * p.y()));
        const auto zInteger = static_cast<int>(std::floor(inv_scale_ * p.z()));

        const bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

        return isEven ? even_->value(u, v, p) : odd_->value(u, v, p);
    }

private:
    double inv_scale_;
    shared_ptr<texture> even_;
    shared_ptr<texture> odd_;
};


#endif //RAY_TRACING_IN_ONE_WEEK_TEXTURES_H
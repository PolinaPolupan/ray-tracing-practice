#ifndef FILM_H
#define FILM_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "math.h"

struct pixel
{
    uint8_t r, g, b;
};

class film
{
public:
    film(const int width, const int height, int spp)
        : width_(width), height_(height), spp_(spp)
    { accumulation_buffer_.resize(width * height, color(0,0,0)); }

    void add_sample(const int index, const color& L)
    {
        double r = L.x();
        double g = L.y();
        double b = L.z();
        if (r != r) r = 0.0;
        if (g != g) g = 0.0;
        if (b != b) b = 0.0;

        accumulation_buffer_[index] += color(r, g, b);
    }

    [[nodiscard]] std::vector<pixel> get_display_buffer() const
    {
        std::vector<pixel> display_buffer(width_ * height_);

        const double scale = (spp_ > 0) ? 1.0 / static_cast<double>(spp_) : 1.0;

        for (int i = 0; i < width_ * height_; i++) {
            color c = accumulation_buffer_[i] * scale;

            vec3d mapped = aces_approx(c);
            double r = std::pow(mapped.x(), 1.0/2.2);
            double g = std::pow(mapped.y(), 1.0/2.2);
            double b = std::pow(mapped.z(), 1.0/2.2);

            display_buffer[i] = {
                static_cast<uint8_t>(255.99 * std::clamp(r, 0.0, 0.999)),
                static_cast<uint8_t>(255.99 * std::clamp(g, 0.0, 0.999)),
                static_cast<uint8_t>(255.99 * std::clamp(b, 0.0, 0.999))
            };
        }
        return display_buffer;
    }

    void write_color(std::ostream& out) const
    {
        auto buf = get_display_buffer();
        for (const auto& p : buf) {
            out << static_cast<int>(p.r) << ' ' << static_cast<int>(p.g) << ' ' << static_cast<int>(p.b) << '\n';
        }
    }

private:
    int width_, height_;
    int spp_;
    std::vector<color> accumulation_buffer_;

    static vec3d aces_approx(vec3d v)
    {
        v *= 0.6f;
        constexpr double a = 2.51f;
        constexpr double b = 0.03f;
        constexpr double c = 2.43f;
        constexpr double d = 0.59f;
        constexpr double e = 0.14f;

        const vec3d numerator = v * (v * a + b);
        const vec3d denominator = v * (v * c + d) + e;

        return {
            numerator.x() / denominator.x(),
            numerator.y() / denominator.y(),
            numerator.z() / denominator.z()
        };
    }
};

#endif //FILM_H
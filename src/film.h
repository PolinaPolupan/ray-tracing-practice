#ifndef FILM_H
#define FILM_H

#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "filtering.h"
#include "math.h"

struct pixel
{
    uint8_t r_, g_, b_;
};

class Film
{
public:
    Film(const int width, const int height, int spp, filter_type ftype = filter_type::box)
        : width_(width), height_(height), spp_(spp)
    {
        accumulation_buffer_.resize(static_cast<int>(width * height), color(0,0,0));
        weight_buffer_.resize(static_cast<int>(width * height), 0.0);

        switch (ftype) {
            case filter_type::box:      filter_ = std::make_unique<BoxFilter>(); break;
            case filter_type::triangle: filter_ = std::make_unique<TriangleFilter>(); break;
            case filter_type::gaussian: filter_ = std::make_unique<GaussianFilter>(); break;
            case filter_type::mitchell:
            default:                   filter_ = std::make_unique<MitchellFilter>(); break;
        }
    }

    void add_sample(int px, int py, const color& l, double dx, double dy)
    {
        double const x = px + dx;
        double const y = py + dy;
        double rad = filter_->radius();

        int x0 = std::floor(x - rad);
        int x1 = std::floor(x + rad);
        int y0 = std::floor(y - rad);
        int y1 = std::floor(y + rad);

        for (int iy = y0; iy <= y1; iy++) {
            if (iy < 0 || iy >= height_) continue;

            for (int ix = x0; ix <= x1; ix++) {
                if (ix < 0 || ix >= width_) continue;

                double wx = filter_->eval(ix + 0.5 - x);
                double wy = filter_->eval(iy + 0.5 - y);
                double w = wx * wy;

                if (w <= 0.0) continue;

                int index = iy * width_ + ix;

                accumulation_buffer_[index] += l * w;
                weight_buffer_[index] += w;
            }
        }
    }

    [[nodiscard]] std::vector<pixel> get_display_buffer() const
    {
        std::vector<pixel> display_buffer(width_ * height_);

        for (int i = 0; i < width_ * height_; i++) {
            color c = accumulation_buffer_[i];

            double weight = weight_buffer_[i];
            if (std::abs(weight) > 1e-6) {
                c /= weight;
            }
            c = {std::max(0.0, c.x()), std::max(0.0, c.y()), std::max(0.0, c.z())};

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
            out << static_cast<int>(p.r_) << ' ' << static_cast<int>(p.g_) << ' ' << static_cast<int>(p.b_) << '\n';
        }
    }

private:
    int width_, height_;
    int spp_;
    std::vector<color> accumulation_buffer_;
    std::vector<double> weight_buffer_;
    std::unique_ptr<Filter> filter_;

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
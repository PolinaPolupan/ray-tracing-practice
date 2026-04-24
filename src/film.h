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

class Filter {
public:
    virtual ~Filter() = default;
    [[nodiscard]] virtual double eval(double x) const = 0;
    [[nodiscard]] virtual double radius() const = 0;
};

class BoxFilter final : public Filter {
public:
    [[nodiscard]] double eval(double x) const override {
        return std::abs(x) <= 0.5 ? 1.0 : 0.0;
    }
    [[nodiscard]] double radius() const override { return 0.5; }
};

class TriangleFilter final : public Filter {
public:
    [[nodiscard]] double eval(double x) const override {
        return std::max(0.0, 1.0 - std::abs(x));
    }
    [[nodiscard]] double radius() const override { return 1.0; }
};

class GaussianFilter final : public Filter {
public:
    GaussianFilter(double r = 2.0, double alpha = 2.0)
        : r_(r), alpha_(alpha), expX_(std::exp(-alpha * r * r)) {}

    [[nodiscard]] double eval(double x) const override {
        return std::max(0.0, std::exp(-alpha_ * x * x) - expX_);
    }
    [[nodiscard]] double radius() const override { return r_; }

private:
    double r_, alpha_, expX_;
};

class MitchellFilter final : public Filter {
public:
    MitchellFilter(double B = 1.0/3.0, double C = 1.0/3.0)
        : B(B), C(C) {}

    [[nodiscard]] double eval(double x) const override {
        x = std::abs(x);
        if (x >= 2.0) return 0.0;
        if (x > 1.0) return f2(x);
        return f1(x);
    }
    [[nodiscard]] double radius() const override { return 2.0; }

private:
    double B, C;

    double f1(double x) const {
        return ((12 - 9*B - 6*C)*(x*x*x)
              + (-18 + 12*B + 6*C)*(x*x)
              + (6 - 2*B)) / 6.0;
    }

    double f2(double x) const {
        return ((-B - 6*C)*(x*x*x)
              + (6*B + 30*C)*(x*x)
              + (-12*B - 48*C)*x
              + (8*B + 24*C)) / 6.0;
    }
};

enum class FilterType { Box, Triangle, Gaussian, Mitchell };

class film
{
public:
    film(const int width, const int height, int spp, FilterType ftype = FilterType::Box)
        : width_(width), height_(height), spp_(spp)
    {
        accumulation_buffer_.resize(width * height, color(0,0,0));
        weight_buffer_.resize(width * height, 0.0);

        switch (ftype) {
            case FilterType::Box:      filter_ = std::make_unique<BoxFilter>(); break;
            case FilterType::Triangle: filter_ = std::make_unique<TriangleFilter>(); break;
            case FilterType::Gaussian: filter_ = std::make_unique<GaussianFilter>(); break;
            case FilterType::Mitchell:
            default:                   filter_ = std::make_unique<MitchellFilter>(); break;
        }
    }

    void add_sample(int px, int py, const color& L, double dx, double dy)
    {
        double x = px + dx;
        double y = py + dy;
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

                accumulation_buffer_[index] += L * w;
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
            out << static_cast<int>(p.r) << ' ' << static_cast<int>(p.g) << ' ' << static_cast<int>(p.b) << '\n';
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
#ifndef FILTERING_H
#define FILTERING_H

class Filter {
public:
    virtual ~Filter() = default;
    [[nodiscard]] virtual auto eval(double x) const -> double = 0;
    [[nodiscard]] virtual auto radius() const -> double = 0;
};

class BoxFilter final : public Filter {
public:
    [[nodiscard]] auto eval(double x) const -> double override {
        return std::abs(x) <= 0.5 ? 1.0 : 0.0;
    }
    [[nodiscard]] auto radius() const -> double override { return 0.5; }
};
class TriangleFilter final : public Filter {
public:
    [[nodiscard]] auto eval(double x) const -> double override {
        return std::max(0.0, 1.0 - std::abs(x));
    }
    [[nodiscard]] auto radius() const -> double override { return 1.0; }
};

class GaussianFilter final : public Filter {
public:
    GaussianFilter(double r = 2.0, double alpha = 2.0)
        : r_(r), alpha_(alpha), expX_(std::exp(-alpha * r * r)) {}

    [[nodiscard]] auto eval(double x) const -> double override {
        return std::max(0.0, std::exp(-alpha_ * x * x) - expX_);
    }
    [[nodiscard]] auto radius() const -> double override { return r_; }

private:
    double r_, alpha_, expX_;
};

class MitchellFilter final : public Filter {
public:
    MitchellFilter(double b = 1.0/3.0, double c = 1.0/3.0)
        : B_(b), C_(c) {}

    [[nodiscard]] auto eval(double x) const -> double override {
        x = std::abs(x);
        if (x >= 2.0) { return 0.0; }
        if (x > 1.0) { return f2(x); }
        return f1(x);
    }
    [[nodiscard]] auto radius() const -> double override { return 2.0; }

private:
    double B_, C_;

    [[nodiscard]] auto f1(double x) const -> double {
        return ((12 - 9*B_ - 6*C_)*(x*x*x)
              + (-18 + 12*B_ + 6*C_)*(x*x)
              + (6 - 2*B_)) / 6.0;
    }

    [[nodiscard]] auto f2(double x) const -> double {
        return ((-B_ - 6*C_)*(x*x*x)
              + (6*B_ + 30*C_)*(x*x)
              + (-12*B_ - 48*C_)*x
              + (8*B_ + 24*C_)) / 6.0;
    }
};

enum class filter_type { box, triangle, gaussian, mitchell };

#endif
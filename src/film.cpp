//
// Created by polup on 11/01/2026.
//

#include "film.h"


double film::linear_to_gamma(const double linear_component) {
    if (linear_component > 0)
        return std::sqrt(linear_component);

    return 0;
}

vec3 aces_approx(vec3 v) {
    v *= 0.6f;
    double a = 2.51f;
    double b = 0.03f;
    double c = 2.43f;
    double d = 0.59f;
    double e = 0.14f;

    // Calculate numerator and denominator vectors separately
    vec3 numerator = v * (v * a + b);      // Works: vec3 * scalar + scalar
    vec3 denominator = v * (v * c + d) + e; // Works: vec3 * scalar + scalar

    // Manually divide component-wise since vec3 / vec3 isn't defined
    return vec3(
        numerator.x() / denominator.x(),
        numerator.y() / denominator.y(),
        numerator.z() / denominator.z()
    );
}

void film::write_color(std::ostream &out, const color &pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Replace NaN components with zero.
    if (r != r) r = 0.0;
    if (g != g) g = 0.0;
    if (b != b) b = 0.0;

    // Apply Tone Mapping
    vec3 mapped = aces_approx(vec3(r, g, b));

    // Gamma Correction (2.2)
    r = std::pow(mapped.x(), 1.0/2.2);
    g = std::pow(mapped.y(), 1.0/2.2);
    b = std::pow(mapped.z(), 1.0/2.2);

    // Translate to [0,255]
    static const interval intensity(0.000, 0.999);
    const int rbyte = static_cast<int>(256 * intensity.clamp(r));
    const int gbyte = static_cast<int>(256 * intensity.clamp(g));
    const int bbyte = static_cast<int>(256 * intensity.clamp(b));

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

//
// Created by polup on 06/01/2026.
//

#ifndef COSINEPDF_H
#define COSINEPDF_H
#include <cmath>



class CosinePdf final : public PDF {
public:
    explicit CosinePdf(const vec3& w) : uvw(w) {}

    [[nodiscard]] double value(const vec3& direction) const override {
        const auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta/pi);
    }

    [[nodiscard]] vec3 generate() const override {
        return uvw.transform(sample_uniform_hemisphere());
    }

private:
    orthonormal_base uvw;
};



#endif //COSINEPDF_H

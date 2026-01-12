//
// Created by polup on 06/01/2026.
//

#ifndef COSINEPDF_H
#define COSINEPDF_H
#include <cmath>

#include "ONB.h"


class CosinePdf final : public PDF {
public:
    explicit CosinePdf(const Vec3& w) : uvw(w) {}

    [[nodiscard]] double value(const Vec3& direction) const override {
        const auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta/pi);
    }

    [[nodiscard]] Vec3 generate() const override {
        return uvw.transform(random_cosine_direction());
    }

private:
    ONB uvw;
};



#endif //COSINEPDF_H

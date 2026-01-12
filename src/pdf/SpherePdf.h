//
// Created by polup on 06/01/2026.
//

#ifndef SPHEREPDF_H
#define SPHEREPDF_H


class SpherePdf final : public PDF {
public:
    SpherePdf() = default;

    [[nodiscard]] double value(const Vec3& direction) const override {
        return 1/(4 * pi);
    }

    [[nodiscard]] Vec3 generate() const override {
        return random_unit_vector();
    }
};



#endif //SPHEREPDF_H

//
// Created by polup on 06/01/2026.
//

#ifndef PDF_H
#define PDF_H


class PDF {
public:
    virtual ~PDF() = default;

    [[nodiscard]] virtual double value(const Vec3& direction) const = 0;
    [[nodiscard]] virtual Vec3 generate() const = 0;
};

#endif //PDF_H

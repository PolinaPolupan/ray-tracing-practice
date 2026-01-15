//
// Created by polup on 06/01/2026.
//

#ifndef PDF_H
#define PDF_H


class PDF {
public:
    virtual ~PDF() = default;

    [[nodiscard]] virtual double value(const vec3& direction) const = 0;
    [[nodiscard]] virtual vec3 generate(const std::shared_ptr<sampler>& sampler) const = 0;
};

#endif //PDF_H

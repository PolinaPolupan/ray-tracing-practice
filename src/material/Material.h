//
// Created by polup on 04/11/2025.
//

#ifndef MATERIAL_H
#define MATERIAL_H
#include <optional>

#include "bsdf.h"
#include "ScatterRecord.h"


class Material {
public:
    virtual ~Material() = default;

    virtual std::unique_ptr<bsdf> get_bsdf(const shape_intersection& rec) const = 0;

    [[nodiscard]] virtual double pdf(const ray& rIn, const shape_intersection& rec, const ray& scattered) const {
        return 0;
    }

    [[nodiscard]] virtual std::optional<ScatterRecord> f(const ray& rIn, const shape_intersection& rec, const std::shared_ptr<sampler>& sampler) const {
        return {};
    }

    [[nodiscard]] virtual color Le(const ray& r_in, const shape_intersection& rec, double u, double v, const point3& p) const {
        return {0,0,0};
    }
};

#endif //MATERIAL_H

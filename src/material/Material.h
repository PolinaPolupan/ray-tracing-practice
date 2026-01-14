//
// Created by polup on 04/11/2025.
//

#ifndef MATERIAL_H
#define MATERIAL_H
#include "ScatterRecord.h"

class Material {
public:
    virtual ~Material() = default;

    [[nodiscard]] virtual double scatteringPdf(const ray& rIn, const HitRecord& rec, const ray& scattered) const {
        return 0;
    }

    [[nodiscard]] virtual bool scatter(const ray& rIn, const HitRecord& rec, const ScatterRecord& sRec) const {
        return false;
    }

    [[nodiscard]] virtual color emitted(const ray& r_in, const HitRecord& rec, double u, double v, const point3d& p) const {
        return {0,0,0};
    }
};

#endif //MATERIAL_H

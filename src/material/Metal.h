//
// Created by polup on 04/11/2025.
//

#ifndef METAL_H
#define METAL_H
#include "Material.h"
#include "hittable/HitRecord.h"


class Metal final : public Material {
public:
    explicit Metal(const color& albedo, const double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1)  {}

    [[nodiscard]] bool scatter(const ray& rIn, const HitRecord& rec, const ScatterRecord& sRec) const override {
        vec3d reflected = reflect(rIn.d(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());

        sRec.attenuation = albedo;
        sRec.pdfPtr = nullptr;
        sRec.skipPdf = true;
        sRec.skipPdfRay = ray(rec.p, reflected, rIn.time());

        return true;
    }

private:
    color albedo;
    double fuzz;
};

#endif //METAL_H

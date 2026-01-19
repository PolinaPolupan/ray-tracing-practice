//
// Created by polup on 04/11/2025.
//

#ifndef DIELECTRIC_H
#define DIELECTRIC_H
#include "Material.h"
#include "sampling.h"

class Dielectric final : public Material {
public:
    explicit Dielectric(const double refraction_index) : refraction_index(refraction_index) {}

    [[nodiscard]] bool scatter(const ray& rIn, const shape_intersection& rec, const ScatterRecord& sRec, const std::shared_ptr<sampler>& sampler) const override {
        sRec.attenuation = color(1.0, 1.0, 1.0);
        sRec.pdfPtr = nullptr;
        sRec.skipPdf = true;
        const double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        const vec3 unit_direction = unit_vector(rIn.d());
        const double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        const double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        const bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > sampler->gen_1d())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        sRec.skipPdfRay = ray(rec.p, direction, rIn.time());

        return true;
    }

private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;

    static double reflectance(const double cosine, const double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

#endif //DIELECTRIC_H

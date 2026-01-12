//
// Created by polup on 04/11/2025.
//

#ifndef LAMBERTIAN_H
#define LAMBERTIAN_H
#include "pdf/CosinePdf.h"

class Texture;

class Lambertian final : public Material {
public:
    explicit Lambertian(const Color& albedo) : tex(make_shared<SolidColor>(albedo)) {}
    explicit Lambertian(const shared_ptr<Texture> &tex) : tex(tex) {}

    [[nodiscard]] bool scatter(const ray& rIn, const HitRecord& rec, const ScatterRecord& sRec) const override {
        sRec.attenuation = tex->value(rec.u, rec.v, rec.p);
        sRec.pdfPtr = make_shared<CosinePdf>(rec.normal);
        sRec.skipPdf = false;
        return true;
    }

    [[nodiscard]] double scatteringPdf(const ray& r_in, const HitRecord& rec, const ray& scattered) const override {
        const auto cos_theta = dot(rec.normal, unit_vector(scattered.d()));
        return cos_theta < 0 ? 0 : cos_theta/pi;
    }

private:
    shared_ptr<Texture> tex;
};

#endif //LAMBERTIAN_H

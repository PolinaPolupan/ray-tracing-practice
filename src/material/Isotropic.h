//
// Created by polup on 03/01/2026.
//

#ifndef ISOTROPIC_H
#define ISOTROPIC_H
#include "Material.h"
#include "pdf/SpherePdf.h"
#include "texture/SolidColor.h"


class Texture;

class Isotropic final : public Material {
public:
    explicit Isotropic(const color& albedo) : tex(make_shared<SolidColor>(albedo)) {}
    explicit Isotropic(const shared_ptr<Texture> &tex) : tex(tex) {}

    [[nodiscard]] bool scatter(const ray& rIn, const HitRecord& rec, const ScatterRecord& sRec, const std::shared_ptr<sampler>& sampler) const override {
        sRec.attenuation = tex->value(rec.u, rec.v, rec.p);
        sRec.pdfPtr = make_shared<SpherePdf>();
        sRec.skipPdf = false;
        return true;
    }

    [[nodiscard]] double scatteringPdf(const ray& r_in, const HitRecord& rec, const ray& scattered) const override {
        return 1 / (4 * pi);
    }

private:
    shared_ptr<Texture> tex;
};

#endif //ISOTROPIC_H

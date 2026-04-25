#ifndef MATERIALS_H
#define MATERIALS_H
#include <memory>

#include "bsdf.h"
#include "shapes.h"
#include "textures.h"


class material
{
public:
    virtual ~material() = default;

    [[nodiscard]] virtual std::unique_ptr<Bsdf> get_bsdf(const shape_intersection& rec) const = 0;

    [[nodiscard]] virtual color le(
        const ray&  /*r_in*/,
        const shape_intersection&  /*rec*/,
        double  /*u*/,
        double  /*v*/,
        const point3d&  /*p*/) const { return {0,0,0}; }
};

class Lambertian final : public material
{
public:
    explicit Lambertian(const color& albedo) : tex_(make_shared<solid_color>(albedo)) {}
    explicit Lambertian(const shared_ptr<texture> &tex) : tex_(tex) {}

    [[nodiscard]] std::unique_ptr<Bsdf> get_bsdf(const shape_intersection& rec) const override
    {
        color const albedo = tex_->value(rec.u, rec.v, rec.p);
        return std::make_unique<LambertianBsdf>(albedo, rec.normal);
    }

private:
    shared_ptr<texture> tex_;
};

class Dielectric final : public material
{
public:
    explicit Dielectric(const double ior): ior_(ior) {}

    [[nodiscard]] std::unique_ptr<Bsdf> get_bsdf(const shape_intersection& rec) const override
    { return std::make_unique<DielectricBsdf>(ior_, rec.normal, rec.front_face); }

private:
    double ior_;
};

class Metal final : public material
{
public:
    explicit Metal(const color& albedo, const double fuzz)
        : albedo_(albedo), fuzz_(fuzz < 1 ? fuzz : 1) {}

    [[nodiscard]] std::unique_ptr<Bsdf> get_bsdf(const shape_intersection& rec) const override
    { return std::make_unique<MetalBsdf>(albedo_, fuzz_, rec.normal); }

private:
    color albedo_;
    double fuzz_;
};

class DiffuseLight final : public material
{
public:
    explicit DiffuseLight(const std::shared_ptr<texture>& tex) : tex_(tex) {}
    explicit DiffuseLight(const color& emit): tex_(make_shared<solid_color>(emit)) {}

    [[nodiscard]] std::unique_ptr<Bsdf> get_bsdf(const shape_intersection& /*rec*/) const override
    { return nullptr; }

    [[nodiscard]] color le(
        const ray& /*unused*/,
        const shape_intersection& rec,
        const double u,
        const double v,
        const point3d& p) const override
    {
        if (!rec.front_face) {
            return {0,0,0};
}
        return tex_->value(u, v, p);
    }

private:
    shared_ptr<texture> tex_;
};

#endif //MATERIALS_H
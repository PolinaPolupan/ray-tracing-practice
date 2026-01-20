#ifndef RAY_TRACING_IN_ONE_WEEK_MATERIALS_H
#define RAY_TRACING_IN_ONE_WEEK_MATERIALS_H
#include <memory>

#include "bsdf.h"
#include "hittable/shape_intersection.h"
#include "texture/SolidColor.h"
#include "texture/Texture.h"


class material {
public:
    virtual ~material() = default;

    [[nodiscard]] virtual std::unique_ptr<bsdf> get_bsdf(const shape_intersection& rec) const = 0;

    [[nodiscard]] virtual color Le(const ray& r_in, const shape_intersection& rec, double u, double v, const point3& p) const {
        return {0,0,0};
    }
};

class lambertian final : public material {
public:
    explicit lambertian(const color& albedo) : tex_(make_shared<SolidColor>(albedo)) {}
    explicit lambertian(const shared_ptr<Texture> &tex) : tex_(tex) {}

    [[nodiscard]] std::unique_ptr<bsdf> get_bsdf(const shape_intersection& rec) const override {
        color albedo = tex_->value(rec.u, rec.v, rec.p);
        return std::make_unique<lambertian_bsdf>(albedo, rec.normal);
    }

private:
    shared_ptr<Texture> tex_;
};

class dielectric final : public material {
public:
    explicit dielectric(const double ior): ior_(ior) {}

    [[nodiscard]] std::unique_ptr<bsdf> get_bsdf(const shape_intersection& rec) const override {
        return std::make_unique<dielectric_bsdf>(ior_, rec.normal, rec.front_face);
    }

private:
    double ior_;
};

class metal final : public material {
public:
    explicit metal(const color& albedo, const double fuzz)
        : albedo_(albedo), fuzz_(fuzz < 1 ? fuzz : 1) {}

    [[nodiscard]] std::unique_ptr<bsdf> get_bsdf(const shape_intersection& rec) const override {
        return std::make_unique<metal_bsdf>(albedo_, fuzz_, rec.normal);
    }

private:
    color albedo_;
    double fuzz_;
};

class diffuse_light final : public material {
public:
    explicit diffuse_light(const std::shared_ptr<Texture>& tex) : tex_(tex) {}
    explicit diffuse_light(const color& emit): tex_(make_shared<SolidColor>(emit)) {}

    [[nodiscard]] std::unique_ptr<bsdf> get_bsdf(const shape_intersection&) const override {
        return nullptr;
    }

    [[nodiscard]] color Le(const ray&, const shape_intersection& rec, const double u, const double v, const point3& p) const override {
        if (!rec.front_face)
            return {0,0,0};
        return tex_->value(u, v, p);
    }

private:
    shared_ptr<Texture> tex_;
};

#endif //RAY_TRACING_IN_ONE_WEEK_MATERIALS_H
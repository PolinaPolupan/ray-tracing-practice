#ifndef LIGHTS_H
#define LIGHTS_H
#include "math.h"
#include "rtw_image.h"
#include "shapes.h"

struct light_li_sample
{
    color li;
    vec3d wi;
    vec3d p_light;
    double pdf{};
};

class light
{
public:
    virtual ~light() = default;
    light() = default;
    explicit light(const vec3d& pos): pos_(pos) {}
    virtual light_li_sample sample_Li(const vec3d& p, const point2d& u) = 0;
    [[nodiscard]] virtual double pdf_Li() const = 0;
    virtual color Le(const vec3d& dir) = 0;
    virtual bool is_infinite() = 0;

protected:
    vec3d pos_;
};

class point_light: public light
{
public:
    point_light(const vec3d& pos, const double scale): light(pos), scale_(scale) {}

    light_li_sample sample_Li(const vec3d& p, const point2d& u) override
    {
        const vec3d d = pos_ - p;
        const vec3d wi = unit_vector(d);

        const double dist2 = std::max(0.001, d.length_squared());
        const color li = (scale_ * vec3d(1.0f)) / dist2;

        return {li, wi, pos_, 1.0};
    }

    [[nodiscard]] double pdf_Li() const override { return 0.0; }

    color Le(const vec3d& dir) override { return scale_; }

    bool is_infinite() override { return false; }

private:
    double scale_;
};

class uniform_infinite_light : public light
{
public:
    uniform_infinite_light(const bounds3d& scene_bounds, const double scale) : scale_(scale)
    {
        center_ = (scene_bounds.p_min + scene_bounds.p_max) / 2;
        radius_ = (scene_bounds.p_max - center_).length();
    }

    light_li_sample sample_Li(const vec3d& p, const point2d& u) override
    {
        // Sample a random direction on the sphere
        const vec3d wi = sample_uniform_sphere(u);

        // Position should be infinitely far in the direction of wi
        const point3d light_pos = p + wi * (2 * radius_);

        return {scale_, wi, light_pos, 1 / (4 * pi)};
    }

    [[nodiscard]] double pdf_Li() const override { return 1 / (4 * pi); }

    bool is_infinite() override { return true; }

    color Le(const vec3d& dir) override { return vec3d(scale_); }

private:
    double scale_;
    point3d center_;
    double radius_;
};

struct sampled_light
{
    std::shared_ptr<light> light_ptr;
    double p;
};

class light_sampler
{
public:
    explicit light_sampler(const std::vector<std::shared_ptr<light>>& lights): lights_(lights) {};

    [[nodiscard]] sampled_light sample(const double u) const
    {
        if (lights_.empty()) return {};

        const int idx = std::min<int>(u * lights_.size(), lights_.size() - 1);
        return {lights_[idx], 1.0 / lights_.size()};
    }

private:
    std::vector<std::shared_ptr<light>> lights_;
};

class environment_light : public light
{
public:
    environment_light(const std::string& image_filename, const bounds3d& scene_bounds, double scale = 1.0)
        : image_(image_filename.c_str()), scale_(scale)
    {
        center_ = (scene_bounds.p_min + scene_bounds.p_max) / 2.0;
        radius_ = (scene_bounds.p_max - center_).length();
    }

    light_li_sample sample_Li(const vec3d& p, const point2d& u) override
    {
        const vec3d wi = sample_uniform_sphere(u);
        const point3d light_pos = p + wi * (2 * radius_);

        return {Le(wi), wi, light_pos, pdf_Li()};
    }

    [[nodiscard]] double pdf_Li() const override { return 1.0 / (4.0 * pi); }

    bool is_infinite() override { return true; }

    color Le(const vec3d& dir) override
    {
        if (image_.width() == 0) return {0, 0, 0};

        const vec3d d = unit_vector(dir);

        const double theta = std::acos(d.y());
        const double phi = std::atan2(-d.z(), d.x()) + pi;

        const double u = phi / (2 * pi);
        const double v = theta / pi;

        const int i = static_cast<int>(u * image_.width());
        const int j = static_cast<int>(v * image_.height());

        const auto pixel = image_.pixelData(i, j);

        return scale_ * color(pixel[0] / 255.0, pixel[1] / 255.0, pixel[2] / 255.0);
    }

private:
    rtw_image image_;
    double scale_;
    point3d center_;
    double radius_;
};

class AreaLight : public light {
public:
    AreaLight(std::shared_ptr<shape> shape, double scale)
        : shape_(std::move(shape)), scale_(scale) {}

    light_li_sample sample_Li(const vec3d& p, const point2d& u) override
    {
        shape_sample s = shape_->sample(u);

        vec3d wi = s.p - p;
        const double dist2 = wi.length_squared();
        wi = unit_vector(wi);

        const double cos_theta = dot(s.n, -wi);

        if (cos_theta <= 0.0) {
            return {
                color(0, 0, 0),
                wi,
                s.p,
                0.0
            };
        }

        const double pdf = s.pdf * dist2 / cos_theta;

        return {
            scale_,
            wi,
            s.p,
            pdf
        };
    }

    [[nodiscard]] double pdf_Li() const override {
        return 0.0;
    }

    color Le(const vec3d&) override {
        return scale_;
    }

    bool is_infinite() override { return false; }

private:
    std::shared_ptr<shape> shape_;
    double scale_;
};

#endif //LIGHTS_H
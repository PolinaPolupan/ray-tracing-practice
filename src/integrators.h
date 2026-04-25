#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include <functional>

#include "accel.h"
#include "cameras.h"
#include "lights.h"
#include "math.h"
#include "ray.h"
#include "sampling.h"
#include "shapes.h"

class ray;


class Integrator {
public:
    virtual ~Integrator() = default;
    explicit Integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::vector<std::shared_ptr<light>>& lights,
        const std::shared_ptr<Accelerator>& accelerator
        ) :
    camera_(camera),
    sampler_(sampler),
    lights_(lights),
    accelerator_(accelerator),
    light_sampler_(lights)
    {
        for (const auto& light: lights_)
        {
            if (light->is_infinite()) { infinite_lights_.push_back(light); }
        }
    }

    using RenderCallback = std::function<void(const std::vector<pixel>&)>;

    void render(const RenderCallback& on_sample_complete = nullptr) const;
    void render_debug(const RenderCallback& on_sample_complete = nullptr) const;
    virtual auto li(ray &r, sampler& samp, int depth) const -> vec3d = 0;

    [[nodiscard]] auto unoccluded(const vec3d& p0, const vec3d& p1, const double time) const -> bool {
        const vec3d dir = p1 - p0;
        const double dist = dir.length();
        const vec3d dir_norm = dir / dist;
        const ray shadow_ray(p0, dir_norm, time);

        const std::optional<shape_intersection> hit =
            accelerator_->intersect(shadow_ray, interval(0.001, dist - 0.001));

        return !hit.has_value();
    }

    [[nodiscard]] auto get_tiles() const -> std::vector<bounds2i>
    {
        std::vector<bounds2i> tiles;
        const bounds2i extent({.x=0, .y=0}, {.x=camera_->image_width, .y=camera_->image_height});

        for (int y = 0; y < camera_->image_height; y += tile_size_)
        {
            for (int x = 0; x < camera_->image_width; x += tile_size_)
            {
                bounds2i tile_bounds({.x=x, .y=y},
                    {.x=std::min(x + tile_size_, camera_->image_width),
                        .y=std::min(y + tile_size_, camera_->image_height)});

                tile_bounds = intersect(tile_bounds, extent);

                if (!tile_bounds.is_empty()) {
                    tiles.push_back(tile_bounds);
                }
            }
        }

        return tiles;
    }

    static auto power_heuristic(const double f_pdf, const double g_pdf) -> double {
        if (sqrt(f_pdf) == infinity) {
            return 1;
        }
        return sqrt(f_pdf) / (sqrt(f_pdf) + sqrt(g_pdf));
    }

    static auto power_heuristic(double nf, double f_pdf, double ng, double g_pdf) -> double {
        double const f = nf * f_pdf;
        double const g = ng * g_pdf;
        return (f * f) / (f * f + g * g);
    }

protected:
    std::shared_ptr<camera> camera_;
    std::shared_ptr<sampler> sampler_;
    std::vector<std::shared_ptr<light>> lights_;
    std::vector<std::shared_ptr<light>> infinite_lights_;
    std::shared_ptr<Accelerator> accelerator_;
    light_sampler light_sampler_;

    int max_depth_ = 10;   // Maximum number of ray bounces into scene
    int tile_size_ = 32;
};

class RandomWalkIntegrator : public Integrator
{
public:
    explicit RandomWalkIntegrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::vector<std::shared_ptr<light>>& lights,
        const std::shared_ptr<Accelerator>& accelerator
        ) : Integrator(camera, sampler, lights, accelerator) {}

    [[nodiscard]] vec3d li(ray &r, sampler& samp, int depth) const override;
};

class PathIntegrator: public Integrator
{
public:
    explicit PathIntegrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::vector<std::shared_ptr<light>>& lights,
        const std::shared_ptr<Accelerator>& accelerator
        ) : Integrator(camera, sampler, lights, accelerator) {}

    [[nodiscard]] vec3d li(ray &r, sampler& samp, int depth) const override;
};

#endif //INTEGRATORS_H

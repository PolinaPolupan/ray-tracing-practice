//
// Created by polup on 11/01/2026.
//

#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include <functional>

#include "cameras.h"
#include "lights.h"
#include "math.h"
#include "ray.h"
#include "sampling.h"
#include "shapes.h"

class ray;
class hittable_list;

class integrator {
public:
    virtual ~integrator() = default;

    explicit integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::shared_ptr<hittable_list>& world,
        const std::vector<std::shared_ptr<light>>& lights
        ): camera_(camera), sampler_(sampler), world_(world), lights_(lights), light_sampler_(lights) {}

    using RenderCallback = std::function<void(const std::vector<pixel>&)>;

    void render(RenderCallback on_sample_complete = nullptr) const;

    virtual vec3 li(ray& r, int depth) const = 0;

    [[nodiscard]] bool unoccluded(const vec3& p0, const vec3& p1, const double time) const {
        const vec3 dir = p1 - p0;
        const double dist = dir.length();
        const vec3 dir_norm = dir / dist;
        const ray shadow_ray(p0, dir_norm, time);

        const auto hit = world_->intersect(shadow_ray, interval(0.001, dist - 0.001));
        return !hit.has_value();
    }

    [[nodiscard]] std::vector<bounds2i> get_tiles() const
    {
        std::vector<bounds2i> tiles;
        const bounds2i extent({0, 0}, {camera_->image_width, camera_->image_height});

        for (int y = 0; y < camera_->image_height; y += tile_size_)
        {
            for (int x = 0; x < camera_->image_width; x += tile_size_)
            {
                bounds2i tile_bounds({x, y},
                    {std::min(x + tile_size_, camera_->image_width), std::min(y + tile_size_, camera_->image_height)});

                tile_bounds = intersect(tile_bounds, extent);

                if (!tile_bounds.is_empty()) {
                    tiles.push_back(tile_bounds);
                }
            }
        }

        return tiles;
    }

protected:
    std::shared_ptr<camera> camera_;
    std::shared_ptr<sampler> sampler_;

    std::shared_ptr<hittable_list> world_;
    std::vector<std::shared_ptr<light>> lights_;
    light_sampler light_sampler_;

    int max_depth = 10;   // Maximum number of ray bounces into scene
    int tile_size_ = 32;
};

class random_walk_integrator : public integrator
{
public:
    explicit random_walk_integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::shared_ptr<hittable_list>& world,
        const std::vector<std::shared_ptr<light>>& lights
        ) : integrator(camera, sampler, world, lights) {}

    [[nodiscard]] vec3 li(ray& r, int depth) const override;
};

class path_integrator: public integrator
{
public:
    explicit path_integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::shared_ptr<hittable_list>& world,
        const std::vector<std::shared_ptr<light>>& lights
        ) : integrator(camera, sampler, world, lights) {}

    [[nodiscard]] vec3 li(ray& r, int depth) const override;
};

#endif //INTEGRATORS_H

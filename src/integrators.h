//
// Created by polup on 11/01/2026.
//

#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include "cameras.h"
#include "math.h"
#include "sampling.h"

class ray;
class hittable_list;

class integrator {
public:
    virtual ~integrator() = default;

    explicit integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::shared_ptr<hittable_list>& world,
        const std::shared_ptr<hittable_list>& lights
        ): camera_(camera), sampler_(sampler), world_(world), lights_(lights) {}

    void render() const;

    virtual vec3 li(const ray& r, int depth) const = 0;

protected:
    std::shared_ptr<camera> camera_;
    std::shared_ptr<sampler> sampler_;

    std::shared_ptr<hittable_list> world_;
    std::shared_ptr<hittable_list> lights_;

    int max_depth = 10;   // Maximum number of ray bounces into scene
};

class random_walk_integrator : public integrator
{
public:
    explicit random_walk_integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::shared_ptr<hittable_list>& world,
        const std::shared_ptr<hittable_list>& lights
        ) : integrator(camera, sampler, world, lights) {}

    [[nodiscard]] vec3 li(const ray& r, int depth) const override;
};

class path_integrator: public integrator
{
public:
    explicit path_integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::shared_ptr<hittable_list>& world,
        const std::shared_ptr<hittable_list>& lights
        ) : integrator(camera, sampler, world, lights) {}

    [[nodiscard]] vec3 li(const ray& r, int depth) const override;
};

#endif //INTEGRATORS_H

//
// Created by polup on 11/01/2026.
//

#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include "cameras.h"
#include "math.h"
#include "sampling.h"

class ray;
class HittableList;

class integrator {
public:
    explicit integrator(
        const std::shared_ptr<camera>&camera,
        const std::shared_ptr<sampler>& sampler,
        const std::shared_ptr<HittableList>& world,
        const std::shared_ptr<HittableList>& lights
        ): camera_(camera), sampler_(sampler), world_(world), lights_(lights) {}

    void render() const;

private:
    [[nodiscard]] vec3 li(const ray& r, int depth) const;

public:
    std::shared_ptr<camera> camera_;
    std::shared_ptr<sampler> sampler_;

    std::shared_ptr<HittableList> world_;
    std::shared_ptr<HittableList> lights_;

    int    max_depth         = 10;   // Maximum number of ray bounces into scene
};




#endif //INTEGRATORS_H

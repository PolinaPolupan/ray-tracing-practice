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
    explicit integrator(const std::shared_ptr<camera>&camera, const std::shared_ptr<sampler>& sampler): camera_(camera), sampler_(sampler) {}
    void init() const;
    void render(const HittableList& world, const HittableList& lights) const;

private:
    [[nodiscard]] vec3 li(const ray& r, int depth, const HittableList& world, const HittableList& lights) const;

public:
    std::shared_ptr<camera> camera_;
    std::shared_ptr<sampler> sampler_;

    int    max_depth         = 10;   // Maximum number of ray bounces into scene
    vec3  background{};               // Scene background color
};




#endif //INTEGRATORS_H

//
// Created by polup on 11/01/2026.
//

#ifndef INTEGRATORS_H
#define INTEGRATORS_H

#include "cameras.h"
#include "math.h"

class ray;
class HittableList;

class integrator {
public:
    explicit integrator(const camera &camera): cam(camera) {}
    void init();
    void render(const HittableList& world, const HittableList& lights);

private:
    [[nodiscard]] vec3 li(const ray& r, int depth, const HittableList& world, const HittableList& lights) const;

public:
    camera cam;
    double pixel_samples_scale = 0;  // Color scale factor for a sum of pixel samples
    int    sqrt_spp = 0;             // Square root of number of samples per pixel

    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene
    vec3  background{};               // Scene background color
};




#endif //INTEGRATORS_H

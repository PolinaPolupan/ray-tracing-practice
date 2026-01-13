//
// Created by polup on 11/01/2026.
//

#include "integrators.h"
#include "hittable/HitRecord.h"
#include "material/Material.h"
#include "material/ScatterRecord.h"
#include "pdf/HittablePdf.h"
#include "pdf/MixturePdf.h"


void integrator::init() {
    cam.init();

    sqrt_spp = static_cast<int>(std::sqrt(samples_per_pixel));
    pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
}

void integrator::render(const HittableList& world, const HittableList& lights) {
    init();

    // Render
    std::cout << "P3\n" << cam.image_width << " " << cam.image_height << "\n255\n";

    for (int j = 0; j < cam.image_height; j++) {
        std::clog << "\rScanlines remaining: " << (cam.image_height - j) << ' ' << std::flush;
        for (int i = 0; i < cam.image_width; i++) {
            Color pixel_color(0,0,0);
            for (int s_j = 0; s_j < sqrt_spp; s_j++) {
                for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                    ray r = cam.gen_ray(i, j, s_i, s_j);
                    pixel_color += ray_color(r, max_depth, world, lights);
                }
            }
            cam.write_color(std::cout, pixel_samples_scale * pixel_color);
        }
    }

    std::clog << "\rDone.                 \n";
}

Color integrator::ray_color(const ray &r, int depth, const HittableList &world, const HittableList &lights) const {
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return {0,0,0};

    HitRecord rec;

    // If the ray hits nothing, return the background color.
    if (!world.intersect(r, interval(0.001, infinity), rec))
        return background;

    ScatterRecord sRec;
    const Color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

    if (!rec.mat->scatter(r, rec, sRec))
        return color_from_emission;

    if (sRec.skipPdf) {
        return sRec.attenuation * ray_color(sRec.skipPdfRay, depth-1, world, lights);
    }

    const auto light_ptr = make_shared<HittablePdf>(lights, rec.p);
    const MixturePdf p(light_ptr, sRec.pdfPtr);

    const auto scattered = ray(rec.p, p.generate(), r.time());
    auto pdf_value = p.value(scattered.d());

    const double scattering_pdf = rec.mat->scatteringPdf(r, rec, scattered);

    const Color sample_color = ray_color(scattered, depth-1, world, lights);
    const Color color_from_scatter =
        (sRec.attenuation * scattering_pdf * sample_color) / pdf_value;

    return color_from_emission + color_from_scatter;
}

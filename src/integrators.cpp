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
    camera_->init();

    sqrt_spp = static_cast<int>(std::sqrt(samples_per_pixel));
    pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
}

void integrator::render(const HittableList& world, const HittableList& lights) {
    init();

    // Render
    std::cout << "P3\n" << camera_->image_width << " " << camera_->image_height << "\n255\n";

    for (int j = 0; j < camera_->image_height; j++) {
        std::clog << "\rScanlines remaining: " << (camera_->image_height - j) << ' ' << std::flush;
        for (int i = 0; i < camera_->image_width; i++) {
            color pixel_color(0,0,0);
            for (int s_j = 0; s_j < sqrt_spp; s_j++) {
                for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                    const point2 u = sampler_->gen_2d();
                    ray r = camera_->gen_ray(sampler_, u, i, j, s_i, s_j);
                    pixel_color += li(r, max_depth, world, lights);
                }
            }
            camera_->write_color(std::cout, pixel_samples_scale * pixel_color);
        }
    }

    std::clog << "\rDone.                 \n";
}

color integrator::li(const ray &r, int depth, const HittableList &world, const HittableList &lights) const {
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return {0,0,0};

    HitRecord rec;

    // If the ray hits nothing, return the background color.
    if (!world.intersect(r, interval(0.001, infinity), rec))
        return background;

    ScatterRecord sRec;
    const color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

    if (!rec.mat->scatter(r, rec, sRec, sampler_))
        return color_from_emission;

    if (sRec.skipPdf) {
        return sRec.attenuation * li(sRec.skipPdfRay, depth-1, world, lights);
    }

    const auto light_ptr = make_shared<HittablePdf>(lights, rec.p);
    const MixturePdf p(light_ptr, sRec.pdfPtr);

    const auto scattered = ray(rec.p, p.generate(sampler_), r.time());
    auto pdf_value = p.value(scattered.d());

    const double scattering_pdf = rec.mat->scatteringPdf(r, rec, scattered);

    const color sample_color = li(scattered, depth-1, world, lights);
    const color color_from_scatter =
        (sRec.attenuation * scattering_pdf * sample_color) / pdf_value;

    return color_from_emission + color_from_scatter;
}

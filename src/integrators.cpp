#include "integrators.h"

#include "materials.h"
#include "shapes.h"


void integrator::render() const
{
    camera_->init();

    // Render
    std::cout << "P3\n" << camera_->image_width << " " << camera_->image_height << "\n255\n";

    for (int j = 0; j < camera_->image_height; j++) {
        std::clog << "\rScanlines remaining: " << (camera_->image_height - j) << ' ' << std::flush;
        for (int i = 0; i < camera_->image_width; i++) {
            color pixel_color(0,0,0);

            sampler_->start_pixel();

            while (sampler_->start_next_sample()) {
                ray r = camera_->gen_ray(sampler_, i, j);
                pixel_color += li(r, max_depth);
            }

            camera_->get_film()->write_color(std::cout, pixel_color / sampler_->get_spp());
        }
    }

    std::clog << "\rDone.                 \n";
}

color random_walk_integrator::li(const ray &r, const int depth) const {
    if (depth <= 0)
        return {0,0,0};

    const auto rec_opt = world_->intersect(r, interval(0.001, infinity));
    if (!rec_opt)
        return {0,0,0};

    const shape_intersection& rec = *rec_opt;

    const color L = rec.mat->Le(r, rec, rec.u, rec.v, rec.p);

    const auto bsdf = rec.mat->get_bsdf(rec);
    if (!bsdf) return L;

    const vec3 wo = -unit_vector(r.d());

    if (bsdf->is_specular()) {
        const auto s = bsdf->sample_f(wo, sampler_->gen_2d());
        return L + s.f * li(ray(rec.p, s.wi, r.time()), depth - 1);
    }

    const frame fr(rec.normal);

    const vec3 wi_local = sample_uniform_hemisphere(sampler_->gen_2d());
    const vec3 wi = fr.from_local(wi_local);
    const color f = bsdf->f(wo, wi);

    const double cos_theta = std::max(0.0, dot(rec.normal, unit_vector(wi)));

    return L + f * li(ray(rec.p, wi, r.time()), depth-1) * cos_theta / (1.0 / (2.0 * pi));
}

color path_integrator::li(const ray &r, const int depth) const {
    if (depth <= 0) return {0,0,0};

    const auto rec_opt = world_->intersect(r, interval(0.001, infinity));
    if (!rec_opt) return {0, 0, 0};

    const shape_intersection& rec = *rec_opt;
    const color L = rec.mat->Le(r, rec, rec.u, rec.v, rec.p);

    const auto bsdf = rec.mat->get_bsdf(rec);
    if (!bsdf) return L;

    const vec3 wo = -unit_vector(r.d());
    vec3 wi;

    if (bsdf->is_specular()) {
        const auto s = bsdf->sample_f(wo, sampler_->gen_2d());
        return L + s.f * li(ray(rec.p, s.wi, r.time()), depth - 1);
    }

    if (sampler_->gen_1d() < 0.5) {
        const vec3 light_vec = lights_->random(rec.p, sampler_);
        wi = unit_vector(light_vec);
    } else {
        wi = bsdf->sample_f(wo, sampler_->gen_2d()).wi;
        wi = unit_vector(wi);
    }

    const double p_light = lights_->pdf(rec.p, wi);
    const double p_bsdf = bsdf->pdf(wo, wi);
    const double pdf_val = 0.5 * p_light + 0.5 * p_bsdf;

    if (pdf_val <= 1e-8) return L;

    const color f_val = bsdf->f(wo, wi);

    const double cos_theta = std::max(0.0, dot(rec.normal, unit_vector(wi)));

    return L + (f_val * li(ray(rec.p, wi, r.time()), depth - 1) * cos_theta) / pdf_val;
}

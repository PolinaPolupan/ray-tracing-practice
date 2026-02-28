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

color random_walk_integrator::li(ray &r, const int depth) const {
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
        r = ray(rec.p, s.wi, r.time());
        return L + s.f * li(r, depth - 1);
    }

    const frame fr(rec.normal);

    const vec3 wi_local = sample_uniform_hemisphere(sampler_->gen_2d());
    const vec3 wi = fr.from_local(wi_local);
    const color f = bsdf->f(wo, wi);

    const double cos_theta = std::max(0.0, dot(rec.normal, unit_vector(wi)));

    r = ray(rec.p, wi, r.time());
    return L + f * li(r, depth-1) * cos_theta / (1.0 / (2.0 * pi));
}

inline double power_heuristic(double pdf_f, double pdf_g) {
    double f2 = pdf_f * pdf_f;
    double g2 = pdf_g * pdf_g;
    return f2 / (f2 + g2);
}

color path_integrator::li(ray &r, const int d) const {
    color L = 0.0f, beta = 1.0f;
    int depth = 0;

    while (beta)
    {
        const auto rec_opt = world_->intersect(r, interval(0.001, infinity));
        if (!rec_opt) return {0, 0, 0};

        // Emissive surfaces
        const shape_intersection& rec = *rec_opt;
        L += beta * rec.mat->Le(r, rec, rec.u, rec.v, rec.p);

        // End path if maximum depth reached
        if (depth++ == max_depth)
            break;

        const auto bsdf = rec.mat->get_bsdf(rec);
        if (!bsdf) return L;

        const vec3 wo = -unit_vector(r.d());
        vec3 wi;

        if (bsdf->is_specular()) {
            const auto s = bsdf->sample_f(wo, sampler_->gen_2d());
            beta *= s.f;
            r = ray(rec.p, s.wi, r.time());
            continue;
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

        if (pdf_val <= 1e-8) break;

        const color f_val = bsdf->f(wo, wi);
        const double cos_theta = std::max(0.0, dot(rec.normal, unit_vector(wi)));
        beta *= f_val * cos_theta / pdf_val;

        r = ray(rec.p, wi, r.time());
    }

    return L;
}

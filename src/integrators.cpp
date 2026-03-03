#include "integrators.h"

#include "materials.h"
#include "shapes.h"


void integrator::render() const
{
    camera_->init();

    std::vector<color> image_buffer(camera_->image_width * camera_->image_height);

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

            const int index = j * camera_->image_width + i;
            image_buffer[index] = pixel_color / sampler_->get_spp();
        }
    }

    for (const auto& pixel : image_buffer) {
        camera_->get_film()->write_color(std::cout, pixel);
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

        if (bsdf->is_specular()) {
            const auto s = bsdf->sample_f(wo, sampler_->gen_2d());
            beta *= s.f;
            r = ray(rec.p, s.wi, r.time());
            continue;
        }

        const auto [light, p] = light_sampler_.sample(sampler_->gen_1d());
        const light_li_sample ls = light->sample_li(rec.p);
        if (unoccluded(rec.p, ls.p_light, rec.t)) {
            L += beta * bsdf->f(wo, ls.wi) * std::max(0.0, dot(rec.normal, ls.wi)) * ls.li / (ls.pdf * p);
        }

        vec3 wi = bsdf->sample_f(wo, sampler_->gen_2d()).wi;
        const double pdf_val = bsdf->pdf(wo, wi);

        if (pdf_val <= 1e-8) break;

        const color f_val = bsdf->f(wo, wi);
        const double cos_theta = std::max(0.0, dot(rec.normal, unit_vector(wi)));
        beta *= f_val * cos_theta / pdf_val;

        r = ray(rec.p, wi, r.time());

        if (depth > 1) {
            double max_component = std::max({beta.x(), beta.y(), beta.z()});
            const double p_continue = std::max(0.05, max_component);
            if (sampler_->gen_1d() > p_continue) {
                break;
            }
            beta /= p_continue;
        }
    }

    return L;
}

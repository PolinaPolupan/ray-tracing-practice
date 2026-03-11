#include "integrators.h"

#include "materials.h"
#include "shapes.h"
#include <execution>


#include <atomic>
#include <chrono>
#include <omp.h>

void integrator::render(RenderCallback on_sample_complete) const
{
    camera_->init();

    std::cout << "P3\n" << camera_->image_width << " "
              << camera_->image_height << "\n255\n";

    const std::vector<bounds2i> tiles = get_tiles();

    std::atomic tiles_done{0};

    using clock = std::chrono::steady_clock;
    auto last_update = clock::now();
    constexpr auto update_interval = std::chrono::milliseconds(500);

#pragma omp parallel
    {
        const auto thread_sampler = sampler_->clone();

#pragma omp for schedule(dynamic, 1)
        for (int t = 0; t < tiles.size(); ++t)
        {
            const bounds2i tile = tiles[t];
            for (point2i pixel : tile)
            {
                thread_sampler->start_pixel();

                color sum(0,0,0);

                while (thread_sampler->start_next_sample())
                {
                    ray r = camera_->gen_ray(*thread_sampler, pixel);
                    sum += li(r, *thread_sampler, max_depth_);
                }

                const int index = pixel.y * camera_->image_width + pixel.x;
                camera_->get_film()->add_sample(index, sum);
            }

            tiles_done.fetch_add(1, std::memory_order_relaxed);

            if (on_sample_complete && omp_get_thread_num() == 0)
            {
                auto now = clock::now();
                if (now - last_update >= update_interval)
                {
                    on_sample_complete(camera_->get_film()->get_display_buffer());
                    last_update = now;
                }
            }
        }
    }

    camera_->get_film()->write_color(std::cout);
}

color random_walk_integrator::li(ray &r, sampler& samp, const int depth) const {
    if (depth <= 0)
        return {0,0,0};

    const auto rec_opt = accelerator_->intersect(r, interval(0.001, infinity));
    if (!rec_opt)
        return {0,0,0};

    const shape_intersection& rec = *rec_opt;

    const color L = rec.mat->Le(r, rec, rec.u, rec.v, rec.p);

    const auto bsdf = rec.mat->get_bsdf(rec);
    if (!bsdf) return L;

    const vec3 wo = -unit_vector(r.d());

    if (bsdf->is_specular()) {
        const auto s = bsdf->sample_f(wo, samp.gen_2d());
        r = ray(rec.p, s.wi, r.time());
        return L + s.f * li(r, samp, depth - 1);
    }

    const frame fr(rec.normal);

    const vec3 wi_local = sample_uniform_hemisphere(samp.gen_2d());
    const vec3 wi = fr.from_local(wi_local);
    const color f = bsdf->f(wo, wi);

    const double cos_theta = std::max(0.0, dot(rec.normal, unit_vector(wi)));

    r = ray(rec.p, wi, r.time());
    return L + f * li(r, samp, depth-1) * cos_theta / (1.0 / (2.0 * pi));
}

color path_integrator::li(ray &r, sampler& samp, int d) const {
    color L = 0.0f, beta = 1.0f;
    int depth = 0;

    while (beta)
    {
        const std::optional<shape_intersection> rec_opt =
            accelerator_->intersect(r, interval(0.001, infinity));
        if (!rec_opt) return {0, 0, 0};

        // Emissive surfaces
        const shape_intersection& rec = *rec_opt;
        L += beta * rec.mat->Le(r, rec, rec.u, rec.v, rec.p);

        // End path if maximum depth reached
        if (depth++ == max_depth_)
            break;

        const auto bsdf = rec.mat->get_bsdf(rec);
        if (!bsdf) return L;

        const vec3 wo = -unit_vector(r.d());

        if (bsdf->is_specular()) {
            const auto s = bsdf->sample_f(wo, samp.gen_2d());
            beta *= s.f;
            r = ray(rec.p, s.wi, r.time());
            continue;
        }

        const auto [light, p] = light_sampler_.sample(samp.gen_1d());
        const light_li_sample ls = light->sample_li(rec.p);
        if (unoccluded(rec.p, ls.p_light, rec.t)) {
            L += beta * bsdf->f(wo, ls.wi) * std::max(0.0, dot(rec.normal, ls.wi)) * ls.li / (ls.pdf * p);
        }

        auto [wi, f, pdf] = bsdf->sample_f(wo, samp.gen_2d());

        if (pdf <= 1e-8) break;

        const double cos_theta = std::max(0.0, dot(rec.normal, wi));
        beta *= f * cos_theta / pdf;

        r = ray(rec.p, wi, r.time());

        if (depth > 3) {
            double max_component = std::max({beta.x(), beta.y(), beta.z()});
            const double p_continue = std::max(0.05, max_component);
            if (samp.gen_1d() > p_continue) {
                break;
            }
            beta /= p_continue;
        }
    }

    return L;
}

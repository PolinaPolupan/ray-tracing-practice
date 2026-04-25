#include "integrators.h"

#include "materials.h"
#include "shapes.h"
#include <execution>


#include <atomic>
#include <chrono>
#include <omp.h>

void Integrator::render(const RenderCallback& on_sample_complete) const
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
            for (point2i const pixel : tile)
            {
                thread_sampler->start_pixel();

                color const sum(0,0,0);

                while (thread_sampler->start_next_sample())
                {
                    point2d const jitter = thread_sampler->gen_2d();

                    ray r = camera_->gen_ray(*thread_sampler, pixel);

                    color const l = li(r, *thread_sampler, max_depth_);

                    camera_->get_film()->add_sample(
                        pixel.x,
                        pixel.y,
                        l,
                        jitter.x,
                        jitter.y
                    );
                }
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

void Integrator::render_debug(const RenderCallback& on_sample_complete) const
{
    camera_->init();

    std::cout << "P3\n" << camera_->image_width << " "
              << camera_->image_height << "\n255\n";

    const std::vector<bounds2i> tiles = get_tiles();

    std::atomic tiles_done{0};

    std::atomic<double> total_image_noise{0.0};

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
            for (point2i const pixel : tile)
            {
                thread_sampler->start_pixel();

                color mean(0,0,0);
                color m2(0,0,0);
                int sample_count = 0;

                while (thread_sampler->start_next_sample())
                {
                    sample_count++;
                    point2d const jitter = thread_sampler->gen_2d();

                    ray r = camera_->gen_ray(*thread_sampler, pixel);

                    color const l = li(r, *thread_sampler, max_depth_);

                    color delta = l - mean;
                    mean += delta / sample_count;
                    color delta2 = l - mean;

                    m2 += color(
                        delta.x() * delta2.x(),
                        delta.y() * delta2.y(),
                        delta.z() * delta2.z()
                    );

                    camera_->get_film()->add_sample(
                        pixel.x,
                        pixel.y,
                        l,
                        jitter.x,
                        jitter.y
                    );
                }

                if (sample_count > 1) {
                    color pixel_variance = m2 / (sample_count - 1);
                    double const pixel_noise = (pixel_variance.x() + pixel_variance.y() + pixel_variance.z()) / 3.0;

                    total_image_noise.store(total_image_noise.load(std::memory_order_relaxed) + pixel_noise, std::memory_order_relaxed);
                }
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

    std::cout << "Average Image Variance: "
              << total_image_noise.load() / (camera_->image_width * camera_->image_height)
              << "\n";
}

color RandomWalkIntegrator::li(ray &r, sampler& samp, const int depth) const {
    if (depth <= 0) {
        return {0,0,0};
}

    const auto rec_opt = accelerator_->intersect(r, interval(0.001, infinity));
    if (!rec_opt) {
        return {0,0,0};
}

    const shape_intersection& rec = *rec_opt;

    const color l = rec.mat->le(r, rec, rec.u, rec.v, rec.p);

    const auto bsdf = rec.mat->get_bsdf(rec);
    if (!bsdf) { return l;
}

    const vec3 wo = -unit_vector(r.d());

    if (bsdf->is_specular()) {
        const auto s = bsdf->sample_f(wo, samp.gen_2d());
        r = ray(rec.p, s.wi_, r.time());
        return l + s.f_ * li(r, samp, depth - 1);
    }

    const frame fr(rec.normal);

    const vec3 wi_local = sample_uniform_hemisphere(samp.gen_2d());
    const vec3 wi = fr.from_local(wi_local);
    const color f = bsdf->f(wo, wi);

    const double cos_theta = std::max(0.0, dot(rec.normal, unit_vector(wi)));

    r = ray(rec.p, wi, r.time());
    return l + f * li(r, samp, depth-1) * cos_theta / (1.0 / (2.0 * pi));
}

color PathIntegrator::li(ray &r, sampler& samp, int  /*d*/) const {
    color l = 0.0f;
    color beta = 1.0f;
    int depth = 0;

    bool specular_bounce = true;
    double last_bsdf_pdf = 1.0;

    while (beta)
    {
        const std::optional<shape_intersection> si =
            accelerator_->intersect(r, interval(0.001, infinity));

        if (!si)
        {
            for (const auto& light: infinite_lights_)
            {
                if (specular_bounce) {
                    l += beta * light->Le(unit_vector(r.d()));
                } else {
                    const double p_l = light_sampler_.sample(samp.gen_1d()).p * light->pdf_Li(r.o(), r.d());
                    const double w_b = power_heuristic(1.0, last_bsdf_pdf, 1.0, p_l);
                    l += beta * w_b * light->Le(unit_vector(r.d()));
                }
            }
            return l;
        }

        const shape_intersection& rec = *si;
        color le = rec.mat->le(r, rec, rec.u, rec.v, rec.p);

        const vec3 wo = -unit_vector(r.d());

        if (le.x() > 0 || le.y() > 0 || le.z() > 0) {
            if (specular_bounce) {
                l += beta * le;
            } else {
                double total_light_pdf = 0.0;

                if (rec.area_light) {
                    const double p_select = 1.0 / infinite_lights_.size();
                    const double pdf_light = rec.area_light->pdf_Li(r.o(), r.d());
                    total_light_pdf = p_select * pdf_light;
                }

                const double w_b = power_heuristic(1.0, last_bsdf_pdf, 1.0, total_light_pdf);
                l += beta * w_b * le;
            }
        }

        if (depth++ == max_depth_) {
            break;
}

        const auto bsdf = rec.mat->get_bsdf(rec);
        if (!bsdf) { return l;
}

        if (bsdf->is_specular()) {
            const auto s = bsdf->sample_f(wo, samp.gen_2d());
            beta *= s.f_;
            r = ray(rec.p, s.wi_, r.time());
            specular_bounce = true;
            continue;
        }

        const auto [light, p] = light_sampler_.sample(samp.gen_1d());
        const light_li_sample ls = light->sample_Li(rec.p, samp.gen_2d());

        if (unoccluded(rec.p, ls.p_light, rec.t)) {
            const double bsdf_pdf = bsdf->pdf(wo, ls.wi);
            const double total_light_pdf = p * ls.pdf;

            const double weight_light = power_heuristic(1.0, total_light_pdf, 1.0, bsdf_pdf);

            l += beta * bsdf->f(wo, ls.wi) * std::max(0.0, dot(rec.normal, ls.wi)) * ls.li * weight_light / total_light_pdf;
        }

        auto [wi, f, pdf] = bsdf->sample_f(wo, samp.gen_2d());

        if (pdf <= 1e-8) { break;
}

        const double cos_theta = std::max(0.0, dot(rec.normal, wi));
        beta *= f * cos_theta / pdf;

        r = ray(rec.p, wi, r.time());

        specular_bounce = false;
        last_bsdf_pdf = pdf;

        if (depth > 3) {
            double const max_component = std::max({beta.x(), beta.y(), beta.z()});
            const double p_continue = std::max(0.05, std::min(1.0, max_component));
            if (samp.gen_1d() > p_continue) {
                break;
            }
            beta /= p_continue;
        }
    }

    return l;
}

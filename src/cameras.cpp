//
// Created by polup on 11/01/2026.
//

#include "cameras.h"

#include "ray.h"
#include "sampling.h"


void camera::init() {
    image_height = static_cast<int>(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    sqrt_spp = static_cast<int>(std::sqrt(samples_per_pixel));
    recip_sqrt_spp = 1.0 / sqrt_spp;

    center = lookfrom;

    // Determine viewport dimensions.
    const auto focal_length = (lookfrom - lookat).length();
    const auto theta = degrees_to_radians(vfov);
    const auto h = std::tan(theta/2);
    const auto viewport_height = 2 * h * focus_dist;
    const auto viewport_width = viewport_height * (static_cast<double>(image_width)/image_height);

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    const vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
    const vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    const auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // Calculate the camera defocus disk basis vectors.
    const auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
}

ray camera::gen_ray(const std::shared_ptr<sampler>& sampler, point2 u, const int i, const int j, const int s_i, const int s_j) const {
    // Construct a camera ray originating from the defocus disk and directed at a randomly
    // sampled point around the pixel location i, j for stratified sample square s_i, s_j.

    const auto offset = sample_square_stratified(u, s_i, s_j, recip_sqrt_spp);
    const auto pixel_sample = pixel00_loc
                      + ((i + offset.x()) * pixel_delta_u)
                      + ((j + offset.y()) * pixel_delta_v);

    auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample(sampler, center, defocus_disk_u, defocus_disk_v);
    const auto ray_direction = pixel_sample - ray_origin;

    return {ray_origin, ray_direction};
}

void camera::write_color(std::ostream &out, const color &pixel_color) const {
    film_->write_color(out, pixel_color);
}



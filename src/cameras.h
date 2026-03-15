#ifndef CAMERA_H
#define CAMERA_H

#include "film.h"
#include "math.h"
#include "sampling.h"

class ray;

class camera
{
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width  = 100;  // Rendered image width in pixel count

    double vfov = 90;  // Vertical view angle (field of view)
    point3d lookfrom = point3d(0,0,0);   // Point camera is looking from
    point3d lookat   = point3d(0,0,-1);  // Point camera is looking at
    vec3d   vup      = vec3d(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    int    image_height = 0;   // Rendered image height
    point3d center;         // Camera center
    point3d pixel00_loc;    // Location of pixel 0, 0
    vec3d   pixel_delta_u;  // Offset to pixel to the right
    vec3d   pixel_delta_v;  // Offset to pixel below
    vec3d   u, v, w;              // Camera frame basis vectors
    vec3d   defocus_disk_u;       // Defocus disk horizontal radius
    vec3d   defocus_disk_v;       // Defocus disk vertical radius

    explicit camera(film* film): film_(film) {}
    void init();
    [[nodiscard]] ray gen_ray(sampler& sampler, point2i pixel) const;
    [[nodiscard]] film* get_film() const { return film_; }

private:
    film* film_;
};

#endif
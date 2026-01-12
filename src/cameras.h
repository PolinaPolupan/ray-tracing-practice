#ifndef CAMERA_H
#define CAMERA_H

#include "film.h"
#include "Vec3.h"

class ray;

class camera {
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width  = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel

    double vfov = 90;  // Vertical view angle (field of view)
    Point3 lookfrom = Point3(0,0,0);   // Point camera is looking from
    Point3 lookat   = Point3(0,0,-1);  // Point camera is looking at
    Vec3   vup      = Vec3(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    int    image_height = 0;   // Rendered image height
    Point3 center;         // Camera center
    Point3 pixel00_loc;    // Location of pixel 0, 0
    Vec3   pixel_delta_u;  // Offset to pixel to the right
    Vec3   pixel_delta_v;  // Offset to pixel below
    Vec3   u, v, w;              // Camera frame basis vectors
    Vec3   defocus_disk_u;       // Defocus disk horizontal radius
    Vec3   defocus_disk_v;       // Defocus disk vertical radius
    int    sqrt_spp = 0;             // Square root of number of samples per pixel
    double recip_sqrt_spp = 0;       // 1 / sqrt_spp

public:
    explicit camera(film* film): film_(film) {}

    void init();

    [[nodiscard]] ray gen_ray(int i, int j, int s_i, int s_j) const;

    void write_color(std::ostream& out, const Color& pixel_color) const;

private:
    [[nodiscard]] Vec3 sample_square_stratified(int s_i, int s_j) const;

    [[nodiscard]] Vec3 sample_square() {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return {random_double() - 0.5, random_double() - 0.5, 0};
    }

    [[nodiscard]] Point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

private:
    film* film_;
};

#endif
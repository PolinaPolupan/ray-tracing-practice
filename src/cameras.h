#ifndef CAMERA_H
#define CAMERA_H

#include "Color.h"
#include "hittable/Hittable.h"
#include "Ray.h"
#include "Vec3.h"
#include "material/Material.h"
#include "pdf/CosinePdf.h"
#include "pdf/HittablePdf.h"
#include "pdf/MixturePdf.h"

class Camera {
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width  = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene
    Color  background;               // Scene background color

    double vfov = 90;  // Vertical view angle (field of view)
    Point3 lookfrom = Point3(0,0,0);   // Point camera is looking from
    Point3 lookat   = Point3(0,0,-1);  // Point camera is looking at
    Vec3   vup      = Vec3(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    void render(const Hittable& world, const Hittable& lights) {
        initialize();

        // Render
        std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                Color pixel_color(0,0,0);
                for (int s_j = 0; s_j < sqrt_spp; s_j++) {
                    for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                        Ray r = getRay(i, j, s_i, s_j);
                        pixel_color += ray_color(r, max_depth, world, lights);
                    }
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

private:
    int    image_height = 0;   // Rendered image height
    double pixel_samples_scale = 0;  // Color scale factor for a sum of pixel samples
    Point3 center;         // Camera center
    Point3 pixel00_loc;    // Location of pixel 0, 0
    Vec3   pixel_delta_u;  // Offset to pixel to the right
    Vec3   pixel_delta_v;  // Offset to pixel below
    Vec3   u, v, w;              // Camera frame basis vectors
    Vec3   defocus_disk_u;       // Defocus disk horizontal radius
    Vec3   defocus_disk_v;       // Defocus disk vertical radius
    int    sqrt_spp = 0;             // Square root of number of samples per pixel
    double recip_sqrt_spp = 0;       // 1 / sqrt_spp

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        sqrt_spp = static_cast<int>(std::sqrt(samples_per_pixel));
        pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
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
        const Vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        const Vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

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

    [[nodiscard]] Ray getRay(const int i, const int j, const int s_i, const int s_j) const {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j for stratified sample square s_i, s_j.

        const auto offset = sample_square_stratified(s_i, s_j);
        const auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        const auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return {ray_origin, ray_direction, ray_time};
    }

    [[nodiscard]] Vec3 sample_square_stratified(const int s_i, const int s_j) const {
        // Returns the vector to a random point in the square sub-pixel specified by grid
        // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

        auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

        return {px, py, 0};
    }

    [[nodiscard]] static Vec3 sampleSquare() {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return {random_double() - 0.5, random_double() - 0.5, 0};
    }

    [[nodiscard]] Point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    [[nodiscard]] Color ray_color(const Ray& r, const int depth, const Hittable& world, const Hittable& lights) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return {0,0,0};

        HitRecord rec;

        // If the ray hits nothing, return the background color.
        if (!world.hit(r, Interval(0.001, infinity), rec))
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

        const auto scattered = Ray(rec.p, p.generate(), r.time());
        auto pdf_value = p.value(scattered.direction());

        const double scattering_pdf = rec.mat->scatteringPdf(r, rec, scattered);

        const Color sample_color = ray_color(scattered, depth-1, world, lights);
        const Color color_from_scatter =
            (sRec.attenuation * scattering_pdf * sample_color) / pdf_value;

        return color_from_emission + color_from_scatter;
    }
};

#endif
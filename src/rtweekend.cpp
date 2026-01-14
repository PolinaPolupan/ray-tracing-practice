#include <chrono>

#include "rtweekend.h"

#include "hittable/shape.h"
#include "cameras.h"
#include "integrators.h"
#include "hittable/Quad.h"
#include "hittable/RotateY.h"
#include "hittable/Sphere.h"
#include "hittable/Translate.h"
#include "material/Dielectric.h"
#include "material/DiffuseLight.h"
#include "material/Lambertian.h"
#include "texture/ImageTexture.h"


void cornell_box() {
    HittableList world;

    auto red   = make_shared<Lambertian>(color(.65, .05, .05));
    auto white = make_shared<Lambertian>(color(.73, .73, .73));
    auto green = make_shared<Lambertian>(color(.12, .45, .15));
    auto light = make_shared<DiffuseLight>(color(15, 15, 15));

    world.add(make_shared<Quad>(point3d(555,0,0), vec3d(0,555,0), vec3d(0,0,555), green));
    world.add(make_shared<Quad>(point3d(0,0,0), vec3d(0,555,0), vec3d(0,0,555), red));
    world.add(make_shared<Quad>(point3d(343, 554, 332), vec3d(-130,0,0), vec3d(0,0,-105), light));
    world.add(make_shared<Quad>(point3d(0,0,0), vec3d(555,0,0), vec3d(0,0,555), white));
    world.add(make_shared<Quad>(point3d(555,555,555), vec3d(-555,0,0), vec3d(0,0,-555), white));
    world.add(make_shared<Quad>(point3d(0,0,555), vec3d(555,0,0), vec3d(0,555,0), white));

    // Box
    shared_ptr<shape> box1 = box(point3d(0,0,0), point3d(165,330,165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, vec3d(265,0,295));
    world.add(box1);

    // Glass Sphere
    auto glass = make_shared<Dielectric>(1.5);
    world.add(make_shared<Sphere>(point3d(190,90,190), 90, glass));

    // Light Sources
    HittableList lights;
    auto empty_material = shared_ptr<Material>();
    lights.add(make_shared<Quad>(point3d(343,554,332), vec3d(-130,0,0), vec3d(0,0,-105), empty_material));
    lights.add(make_shared<Sphere>(point3d(190, 90, 190), 90, empty_material));

    film film;
    camera cam(&film);

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 10;

    cam.vfov     = 40;
    cam.lookfrom = point3d(278, 278, -800);
    cam.lookat   = point3d(278, 278, 0);
    cam.vup      = vec3d(0,1,0);

    cam.defocus_angle = 0;

    integrator integrator(cam);

    integrator.samples_per_pixel = 10;

    integrator.render(world, lights);
}

int main() {
    using clock = std::chrono::high_resolution_clock;

    const auto start = clock::now();

    cornell_box();

    const auto end = clock::now();

    const std::chrono::duration<double> elapsed = end - start;
    std::cerr << "Render time: " << elapsed.count() << " seconds\n";
}

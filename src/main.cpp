#include <chrono>

#include "constants.h"

#include "cameras.h"
#include "integrators.h"
#include "materials.h"
#include "shapes.h"
#include "lights.h"
#include <SDL3/SDL.h>

SDL_Renderer* g_renderer = nullptr;
SDL_Texture* g_texture = nullptr;
int g_width = 200;
int g_height = 200;

void update_display(const std::vector<pixel>& buffer) {
    std::vector<uint32_t> pixels(g_width * g_height);

    for (int i = 0; i < g_width * g_height; i++) {
        pixel c = buffer[i];
        pixels[i] = (255 << 24) | (c.b << 16) | (c.g << 8) | c.r;
    }

    SDL_UpdateTexture(g_texture, nullptr, pixels.data(), g_width * sizeof(uint32_t));
    SDL_RenderClear(g_renderer);
    SDL_RenderTexture(g_renderer, g_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);

    // Handle events to keep window responsive
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            exit(0);  // Or set a flag to stop rendering
        }
    }
}

void cornell_box() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    // Box
    shared_ptr<shape> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    // Glass Sphere
    auto glass = make_shared<dielectric>(3.0);
    world.add(make_shared<sphere>(point3(190,90,190), 90, glass));

    auto empty_material = shared_ptr<material>();

    const auto samp = std::make_shared<stratified_sampler>(10);

    film film(g_width, g_height, samp->get_spp());
    const auto cam = std::make_shared<camera>(&film);

    cam->aspect_ratio      = 1.0;
    cam->image_width       = g_width;

    cam->vfov     = 40;
    cam->lookfrom = point3(278, 278, -800);
    cam->lookat   = point3(278, 278, 0);
    cam->vup      = vec3(0,1,0);

    cam->defocus_angle = 0;

    std::vector<std::shared_ptr<light>> lights;
    lights.push_back(std::make_shared<point_light>(vec3(278, 500, 278), 100000.0));

    auto bvh_root = std::make_shared<hittable_list>();
    bvh_root->add(std::make_shared<bvh_node>(world));

    const auto integrator_ptr = std::make_shared<path_integrator>(cam, samp, bvh_root, lights);

    integrator_ptr->render(update_display);
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Ray Tracer", g_width, g_height, 0);
    if (!window) return 1;

    g_renderer = SDL_CreateRenderer(window, "software");
    if (!g_renderer) return 1;

    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
    if (!g_texture) return 1;

    using clock = std::chrono::high_resolution_clock;

    const auto start = clock::now();

    cornell_box();

    const auto end = clock::now();

    const std::chrono::duration<double> elapsed = end - start;
    std::cerr << "Render time: " << elapsed.count() << " seconds\n";

    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

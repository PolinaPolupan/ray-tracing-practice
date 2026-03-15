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
int g_width = 600;
int g_height = 600;

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
    std::vector<std::shared_ptr<shape>> world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));

    auto append_mesh = [&](std::vector<std::shared_ptr<shape>> mesh) {
        world.insert(world.end(), mesh.begin(), mesh.end());
    };

    append_mesh(make_quad_mesh(point3d(555,0,0),       vec3d(0,555,0),    vec3d(0,0,555),   green)); // Right
    append_mesh(make_quad_mesh(point3d(0,0,0),         vec3d(0,555,0),    vec3d(0,0,555),   red));   // Left
    append_mesh(make_quad_mesh(point3d(0,0,0),         vec3d(555,0,0),    vec3d(0,0,555),   white)); // Floor
    append_mesh(make_quad_mesh(point3d(555,555,555),   vec3d(-555,0,0),   vec3d(0,0,-555),  white)); // Ceiling
    append_mesh(make_quad_mesh(point3d(0,0,555),       vec3d(555,0,0),    vec3d(0,555,0),   white)); // Back

    auto box1_mesh = box(point3d(0,0,0), point3d(165,330,165), white);

    for (auto& s : box1_mesh) {
        std::shared_ptr<shape> obj = s;
        obj = std::make_shared<RotateY>(obj, 15);
        obj = std::make_shared<translate>(obj, vec3d(265,0,295));
        world.push_back(obj);
    }

    // Glass Sphere
    auto glass = make_shared<dielectric>(3.0);
    world.push_back(make_shared<sphere>(point3d(190,90,190), 90, glass));

    auto empty_material = shared_ptr<material>();

    const auto samp = std::make_shared<stratified_sampler>(100);

    film film(g_width, g_height, samp->get_spp());
    const auto cam = std::make_shared<camera>(&film);

    cam->aspect_ratio      = 1.0;
    cam->image_width       = g_width;

    cam->vfov     = 40;
    cam->lookfrom = point3d(278, 278, -800);
    cam->lookat   = point3d(278, 278, 0);
    cam->vup      = vec3d(0,1,0);

    cam->defocus_angle = 0;

    std::vector<std::shared_ptr<light>> lights;
    lights.push_back(std::make_shared<point_light>(vec3d(278, 500, 278), 100000.0));

    std::shared_ptr<accelerator> accelerator = std::make_shared<bvh>(world);

    lights.push_back(std::make_shared<uniform_infinite_light>(accelerator->bounds(), 0.2));

    const auto integrator_ptr = std::make_shared<path_integrator>(cam, samp, lights, accelerator);

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

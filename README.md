# Ray Tracing Practice
A small physically-based renderer written in C++.
This engine supports:
- Path tracing integrator
- Tile-based multithreaded rendering
- BVH acceleration
- Stratified sampling
- Simple materials and textures
- PPM image output

The renderer initially renders a Cornell Box scene.
Support for loading user-defined scenes will be added in the future.
## Requirements:
- C++20 compiler
- CMake ≥ 3.13
- OpenMP support (GCC, Clang, or MSVC)
## Installation
Clone the repository with submodules:
```bash
git clone --recursive https://github.com/PolinaPolupan/ray-tracing-practice.git
cd ray-tracing-practice
cmake -S . -B build
cmake --build build
```
## Running
The executable will be in build/ (or build/Debug/ on Windows).
```bash
# Linux/macOS
./build/RayTracer > output.ppm

# Windows
.\build\Debug\RayTracer.exe > output.ppm
```
The renderer outputs an image in **PPM format**.

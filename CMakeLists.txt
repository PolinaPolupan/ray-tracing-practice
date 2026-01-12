# CMakeList.txt : CMake project for Ray Tracing In One Week, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Ray Tracing In One Week")

# Add source to this project's executable.
add_executable (inOneWeekend
        src/rtweekend.cpp
        src/Vec3.h
        src/Color.h
        src/ray.h
        src/hittable/shape.h
        src/hittable/Sphere.h
        src/rtweekend.h
        src/Interval.h
        src/material/Material.h
        src/material/Lambertian.h
        src/material/Metal.h
        src/material/Dielectric.h
        src/AABB.h
        src/hittable/BVHNode.h
        src/texture/Texture.h
        src/texture/SolidColor.h
        src/texture/CheckerTexture.h
        src/RtwImage.h
        src/texture/ImageTexture.h
        src/Perlin.h
        src/texture/NoiseTexture.h
        src/hittable/Quad.h
        src/material/DiffuseLight.h
        src/hittable/HitRecord.h
        src/hittable/HittableList.h
        src/hittable/Translate.h
        src/hittable/RotateY.h
        src/material/Isotropic.h
        src/hittable/ConstantMedium.h
        src/ONB.h
        src/pdf/PDF.h
        src/pdf/SpherePdf.h
        src/pdf/CosinePdf.h
        src/pdf/HittablePdf.h
        src/pdf/MixturePdf.h
        src/material/ScatterRecord.h
        src/integrators.cpp
        src/integrators.h
        src/film.cpp
        src/film.h
        src/cameras.cpp
        src/cameras.h)

target_include_directories(inOneWeekend PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET inOneWeekend PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

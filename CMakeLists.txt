# CMakeList.txt : CMake project for Ray Tracing In One Week, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Ray tracing practice")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

# Add source to this project's executable.
add_executable (RayTracer
        src/main.cpp
        src/ray.h
        src/constants.h
        src/rtw_image.h
        src/integrators.cpp
        src/integrators.h
        src/film.h
        src/cameras.cpp
        src/cameras.h
        src/sampling.cpp
        src/sampling.h
        src/math.cpp
        src/math.h
        src/bsdf.cpp
        src/bsdf.h
        src/materials.h
        src/textures.cpp
        src/textures.h
        src/rtw_image.cpp
        src/shapes.cpp
        src/shapes.h
        src/lights.h
        src/lights.h
        src/constants.h
        src/constants.h
        src/accel.h)

target_include_directories(RayTracer PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/external
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RayTracer PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

# Link to the actual SDL3 library.
target_link_libraries(RayTracer PRIVATE SDL3::SDL3)
find_package(OpenMP REQUIRED)
target_link_libraries(RayTracer PRIVATE OpenMP::OpenMP_CXX)
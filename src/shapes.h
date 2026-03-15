#ifndef SHAPES_H
#define SHAPES_H
#include <algorithm>
#include <optional>

#include "math.h"
#include "ray.h"
#include "sampling.h"

class material;

class shape_intersection
{
public:
    point3d p;
    vec3d normal;
    shared_ptr<material> mat;
    double t{};
    double u{};
    double v{};
    bool front_face{};

    void set_face_normal(const ray& r, const vec3d& outward_normal)
    {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.d(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class shape
{
public:
    virtual ~shape() = default;
    [[nodiscard]] virtual std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const = 0;
    [[nodiscard]] virtual bounds3d bounds() const = 0;
    [[nodiscard]] virtual double pdf(const point3d& origin, const vec3d& direction) const {
        return 0.0;
    }
    [[nodiscard]] virtual vec3d random(const point3d& origin, const std::shared_ptr<sampler>& sampler) const {
        return {1,0,0};
    }
};

struct triangle_mesh
{
    std::vector<point3d> p;
    std::vector<vec3d> n;
    std::vector<int> indices;
    std::shared_ptr<material> mat;

    triangle_mesh() = default;
};

class triangle final : public shape
{
public:
    triangle(const std::shared_ptr<triangle_mesh>& mesh, const int tri_index): tri_index(tri_index), mesh(mesh) {}

    bounds3d bounds() const override
    {
        const int i0 = mesh->indices[tri_index * 3 + 0];
        const int i1 = mesh->indices[tri_index * 3 + 1];
        const int i2 = mesh->indices[tri_index * 3 + 2];

        const point3d& p0 = mesh->p[i0];
        const point3d& p1 = mesh->p[i1];
        const point3d& p2 = mesh->p[i2];

        const double min_x = std::min({p0.x(), p1.x(), p2.x()});
        const double min_y = std::min({p0.y(), p1.y(), p2.y()});
        const double min_z = std::min({p0.z(), p1.z(), p2.z()});

        const double max_x = std::max({p0.x(), p1.x(), p2.x()});
        const double max_y = std::max({p0.y(), p1.y(), p2.y()});
        const double max_z = std::max({p0.z(), p1.z(), p2.z()});

        return bounds3d(point3d(min_x, min_y, min_z), point3d(max_x, max_y, max_z));
    }

    [[nodiscard]] std::optional<shape_intersection>
    intersect(const ray& r, interval ray_t) const override;

private:
    int tri_index = 0;
    std::shared_ptr<triangle_mesh> mesh;
};

class quad final : public shape {
public:
    quad(const point3d& Q, const vec3d& u, const vec3d& v, const std::shared_ptr<material> &mat);
    [[nodiscard]] bounds3d bounds() const override { return bbox; }
    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] double pdf(const point3d& origin, const vec3d& direction) const override;
    [[nodiscard]] vec3d random(const point3d& origin, const std::shared_ptr<sampler>& sampler) const override
    {
        const auto p = Q + (sampler->gen_1d() * u) + (sampler->gen_1d() * v);
        return p - origin;
    }
    void bounding_box();
    static bool is_interior(double a, double b, shape_intersection& rec);

private:
    point3d Q;
    vec3d u, v;
    vec3d w;
    shared_ptr<material> mat;
    bounds3d bbox;
    vec3d normal;
    double D;
    double area;
};

class RotateY final : public shape {
public:
    RotateY(const shared_ptr<shape>& object, double angle);
    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] bounds3d bounds() const override { return bbox; }

private:
    shared_ptr<shape> object;
    double sin_theta;
    double cos_theta;
    bounds3d bbox;
};

class sphere final : public shape {
public:
    // Stationary Sphere
    sphere(const point3d& static_center, double radius, const shared_ptr<material> &mat)
      : center(static_center, vec3d(0,0,0)), radius(std::fmax(0,radius)), mat(mat)
    {
        const auto rvec = vec3d(radius, radius, radius);
        bbox = bounds3d(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    sphere(const point3d& center1, const point3d& center2, const double radius, const shared_ptr<material> &mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat)
    {
        const auto rvec = vec3d(radius, radius, radius);
        const bounds3d box1(center.at(0) - rvec, center.at(0) + rvec);
        const bounds3d box2(center.at(1) - rvec, center.at(1) + rvec);
        bbox = bounds3d(box1, box2);
    }

    [[nodiscard]] bounds3d bounds() const override { return bbox; }
    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] double pdf(const point3d& origin, const vec3d& direction) const override;
    [[nodiscard]] vec3d random(const point3d& origin, const std::shared_ptr<sampler>& sampler) const override;

private:
    ray center;
    double radius;
    shared_ptr<material> mat;
    bounds3d bbox;

    static void get_sphere_uv(const point3d& p, double& u, double& v);
};

class translate final : public shape
{
public:
    translate(const shared_ptr<shape>& object, const vec3d& offset): _object(object), _offset(offset) {
        _bbox = object->bounds() + offset;
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] bounds3d bounds() const override { return _bbox; }

private:
    shared_ptr<shape> _object;
    vec3d _offset;
    bounds3d _bbox;
};

std::vector<std::shared_ptr<shape>>
box(const point3d& a, const point3d& b, const std::shared_ptr<material>& mat);

std::vector<std::shared_ptr<shape>>
make_quad_mesh(const point3d& Q, const vec3d& u, const vec3d& v, const std::shared_ptr<material>& mat);

std::shared_ptr<triangle_mesh> load_obj(
    const std::string& filename,
    const point3d& offset,
    double scale,
    const std::shared_ptr<material>& mat
);

std::vector<std::shared_ptr<shape>> make_mesh_triangles(
    const std::shared_ptr<triangle_mesh>& mesh_ptr
);

#endif //SHAPES_H
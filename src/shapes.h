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
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t{};
    double u{};
    double v{};
    bool front_face{};

    void set_face_normal(const ray& r, const vec3& outward_normal)
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
    [[nodiscard]] virtual bounds3 bounds() const = 0;
    [[nodiscard]] virtual double pdf(const point3& origin, const vec3& direction) const {
        return 0.0;
    }
    [[nodiscard]] virtual vec3 random(const point3& origin, const std::shared_ptr<sampler>& sampler) const {
        return {1,0,0};
    }
};

struct triangle_mesh
{
    std::vector<point3> p;
    std::vector<vec3> n;
    std::vector<int> indices;
    std::shared_ptr<material> mat;

    triangle_mesh() = default;
};

class triangle final : public shape
{
public:
    triangle(const std::shared_ptr<triangle_mesh>& mesh, const int tri_index): tri_index(tri_index), mesh(mesh) {}

    [[nodiscard]] bounds3 bounds() const override
    {
        const int i0 = mesh->indices[tri_index * 3 + 0];
        const int i1 = mesh->indices[tri_index * 3 + 1];
        const int i2 = mesh->indices[tri_index * 3 + 2];

        const point3& p0 = mesh->p[i0];
        const point3& p1 = mesh->p[i1];
        const point3& p2 = mesh->p[i2];

        bounds3 box(p0, p1);
        box = bounds3(box, bounds3(p2, p2));
        return box;
    }

    [[nodiscard]] std::optional<shape_intersection>
    intersect(const ray& r, interval ray_t) const override;

private:
    int tri_index = 0;
    std::shared_ptr<triangle_mesh> mesh;
};

class quad final : public shape {
public:
    quad(const point3& Q, const vec3& u, const vec3& v, const std::shared_ptr<material> &mat);
    [[nodiscard]] bounds3 bounds() const override { return bbox; }
    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] double pdf(const point3& origin, const vec3& direction) const override;
    [[nodiscard]] vec3 random(const point3& origin, const std::shared_ptr<sampler>& sampler) const override
    {
        const auto p = Q + (sampler->gen_1d() * u) + (sampler->gen_1d() * v);
        return p - origin;
    }
    void bounding_box();
    static bool is_interior(double a, double b, shape_intersection& rec);

private:
    point3 Q;
    vec3 u, v;
    vec3 w;
    shared_ptr<material> mat;
    bounds3 bbox;
    vec3 normal;
    double D;
    double area;
};

class RotateY final : public shape {
public:
    RotateY(const shared_ptr<shape>& object, double angle);
    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> object;
    double sin_theta;
    double cos_theta;
    bounds3 bbox;
};

class sphere final : public shape {
public:
    // Stationary Sphere
    sphere(const point3& static_center, double radius, const shared_ptr<material> &mat)
      : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat)
    {
        const auto rvec = vec3(radius, radius, radius);
        bbox = bounds3(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    sphere(const point3& center1, const point3& center2, const double radius, const shared_ptr<material> &mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat)
    {
        const auto rvec = vec3(radius, radius, radius);
        const bounds3 box1(center.at(0) - rvec, center.at(0) + rvec);
        const bounds3 box2(center.at(1) - rvec, center.at(1) + rvec);
        bbox = bounds3(box1, box2);
    }

    [[nodiscard]] bounds3 bounds() const override { return bbox; }
    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] double pdf(const point3& origin, const vec3& direction) const override;
    [[nodiscard]] vec3 random(const point3& origin, const std::shared_ptr<sampler>& sampler) const override;

private:
    ray center;
    double radius;
    shared_ptr<material> mat;
    bounds3 bbox;

    static void get_sphere_uv(const point3& p, double& u, double& v);
};

class translate final : public shape
{
public:
    translate(const shared_ptr<shape>& object, const vec3& offset): _object(object), _offset(offset) {
        _bbox = object->bounds() + offset;
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;
    [[nodiscard]] bounds3 bounds() const override { return _bbox; }

private:
    shared_ptr<shape> _object;
    vec3 _offset;
    bounds3 _bbox;
};

std::vector<std::shared_ptr<shape>>
box(const point3& a, const point3& b, const std::shared_ptr<material>& mat);

std::vector<std::shared_ptr<shape>>
make_quad_mesh(const point3& Q, const vec3& u, const vec3& v, const std::shared_ptr<material>& mat);

#endif //SHAPES_H
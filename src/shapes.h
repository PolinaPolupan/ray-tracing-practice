//
// Created by polup on 20/01/2026.
//

#ifndef RAY_TRACING_IN_ONE_WEEK_SHAPES_H
#define RAY_TRACING_IN_ONE_WEEK_SHAPES_H
#include <algorithm>
#include <optional>

#include "math.h"
#include "ray.h"
#include "sampling.h"

class material;

class shape_intersection {
public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t{};
    double u{};
    double v{};
    bool front_face{};

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.d(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class shape {
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

    [[nodiscard]] bounds3 bounds() const override {
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

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override {
        const int i0 = mesh->indices[tri_index * 3 + 0];
        const int i1 = mesh->indices[tri_index * 3 + 1];
        const int i2 = mesh->indices[tri_index * 3 + 2];

        const point3& v0 = mesh->p[i0];
        const point3& v1 = mesh->p[i1];
        const point3& v2 = mesh->p[i2];

        const vec3 edge1 = v1 - v0;
        const vec3 edge2 = v2 - v0;
        const vec3 h = cross(r.d(), edge2);
        const double a = dot(edge1, h);

        if (a > -1e-8 && a < 1e-8) return {};

        const double f = 1.0 / a;
        const vec3 s = r.o() - v0;
        const double u = f * dot(s, h);
        if (u < 0.0 || u > 1.0) return {};

        const vec3 q = cross(s, edge1);
        const double v = f * dot(r.d(), q);
        if (v < 0.0 || u + v > 1.0) return {};

        const double t = f * dot(edge2, q);
        if (!ray_t.contains(t)) return {};

        shape_intersection rec;
        rec.t = t;
        rec.p = r.at(t);
        rec.mat = mesh->mat;
        rec.set_face_normal(r, unit_vector(cross(edge1, edge2)));

        return rec;
    }

private:
    int tri_index = 0;
    std::shared_ptr<triangle_mesh> mesh;
};

class quad final : public shape {
public:
    quad(const point3& Q, const vec3& u, const vec3& v, const std::shared_ptr<material> &mat);

    void bounding_box();

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;

    static bool is_interior(double a, double b, shape_intersection& rec);

    [[nodiscard]] double pdf(const point3& origin, const vec3& direction) const override;

    [[nodiscard]] vec3 random(const point3& origin, const std::shared_ptr<sampler>& sampler) const override {
        const auto p = Q + (sampler->gen_1d() * u) + (sampler->gen_1d() * v);
        return p - origin;
    }

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

class hittable_list final : public shape {
public:
    std::vector<shared_ptr<shape>> objects;

    hittable_list() = default;
    explicit hittable_list(const shared_ptr<shape>& object) { add(object); }

    auto begin() { return objects.begin(); }
    auto end() { return objects.end(); }

    [[nodiscard]] auto begin() const { return objects.begin(); }
    [[nodiscard]] auto end() const { return objects.end(); }

    void clear() { objects.clear(); }

    void add(const shared_ptr<shape>& object) {
        objects.push_back(object);
        bbox = bounds3(bbox, object->bounds());
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

    [[nodiscard]] double pdf(const point3& origin, const vec3& direction) const override;

    [[nodiscard]] vec3 random(const point3& origin, const std::shared_ptr<sampler>& sampler) const override;

private:
    bounds3 bbox;
};

shared_ptr<hittable_list> box(const point3& a, const point3& b, const shared_ptr<material>& mat);

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
      : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = vec3(radius, radius, radius);
        bbox = bounds3(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    sphere(const point3& center1, const point3& center2, const double radius, const shared_ptr<material> &mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {
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

class translate final : public shape {
public:
    translate(const shared_ptr<shape>& object, const vec3& offset): object(object), offset(offset) {
        bbox = object->bounds() + offset;
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> object;
    vec3 offset;
    bounds3 bbox;
};

class bvh_node final : public shape {
public:
    explicit bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    bvh_node(std::vector<shared_ptr<shape>>& objects, size_t start, size_t end);

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> left;
    shared_ptr<shape> right;
    bounds3 bbox;

    static bool box_compare(const shared_ptr<shape>& a, const shared_ptr<shape> &b, const int axis_index) {
        const auto a_axis_interval = a->bounds().axis_interval(axis_index);
        const auto b_axis_interval = b->bounds().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<shape> &a, const shared_ptr<shape> &b) {
        return box_compare(a, b, 2);
    }
};

inline std::shared_ptr<hittable_list> make_quad_mesh(const point3& Q, const vec3& u, const vec3& v, const std::shared_ptr<material>& mat) {
    auto mesh = std::make_shared<triangle_mesh>();
    mesh->mat = mat;

    point3 p0 = Q;
    point3 p1 = Q + u;
    point3 p2 = Q + u + v;
    point3 p3 = Q + v;

    mesh->p = { p0, p1, p2, p3 };

    mesh->indices = { 0, 1, 2, 0, 2, 3 };

    vec3 normal = unit_vector(cross(u, v));

    mesh->n = { normal, normal, normal, normal };

    auto list = std::make_shared<hittable_list>();
    list->add(std::make_shared<triangle>(mesh, 0));
    list->add(std::make_shared<triangle>(mesh, 1));

    return list;
}

inline std::shared_ptr<hittable_list> box(const point3& a, const point3& b, const std::shared_ptr<material>& mat)
{
    auto sides = std::make_shared<hittable_list>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    const auto min = point3(std::fmin(a.x(),b.x()), std::fmin(a.y(),b.y()), std::fmin(a.z(),b.z()));
    const auto max = point3(std::fmax(a.x(),b.x()), std::fmax(a.y(),b.y()), std::fmax(a.z(),b.z()));

    const auto dx = vec3(max.x() - min.x(), 0, 0);
    const auto dy = vec3(0, max.y() - min.y(), 0);
    const auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(make_quad_mesh(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(make_quad_mesh(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(make_quad_mesh(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(make_quad_mesh(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(make_quad_mesh(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(make_quad_mesh(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}


#endif //RAY_TRACING_IN_ONE_WEEK_SHAPES_H
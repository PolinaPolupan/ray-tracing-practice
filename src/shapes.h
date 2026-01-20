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

class Quad final : public shape {
public:
    Quad(const point3& Q, const vec3& u, const vec3& v, const std::shared_ptr<material> &mat);

    void bounding_box();

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;

    static bool isInterior(double a, double b, shape_intersection& rec);

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

class HittableList final : public shape {
public:
    std::vector<shared_ptr<shape>> objects;

    HittableList() = default;
    explicit HittableList(const shared_ptr<shape>& object) { add(object); }

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

shared_ptr<HittableList> box(const point3& a, const point3& b, const shared_ptr<material>& mat);

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

class Sphere final : public shape {
public:
    // Stationary Sphere
    Sphere(const point3& static_center, double radius, const shared_ptr<material> &mat)
      : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = vec3(radius, radius, radius);
        bbox = bounds3(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    Sphere(const point3& center1, const point3& center2, const double radius, const shared_ptr<material> &mat)
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

class Translate final : public shape {
public:
    Translate(const shared_ptr<shape>& object, const vec3& offset): object(object), offset(offset) {
        bbox = object->bounds() + offset;
    }

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, interval ray_t) const override;

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> object;
    vec3 offset;
    bounds3 bbox;
};

class BVHNode final : public shape {
public:
    explicit BVHNode(HittableList list) : BVHNode(list.objects, 0, list.objects.size()) {
        // There's a C++ subtlety here. This constructor (without span indices) creates an
        // implicit copy of the hittable list, which we will modify. The lifetime of the copied
        // list only extends until this constructor exits. That's OK, because we only need to
        // persist the resulting bounding volume hierarchy.
    }

    BVHNode(std::vector<shared_ptr<shape>>& objects, size_t start, size_t end);

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

#endif //RAY_TRACING_IN_ONE_WEEK_SHAPES_H
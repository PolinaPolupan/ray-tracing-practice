#ifndef SPHERE_H
#define SPHERE_H
#include "sampling.h"


class Sphere final : public shape {
public:
    // Stationary Sphere
    Sphere(const point3& static_center, double radius, const shared_ptr<Material> &mat)
      : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = vec3(radius, radius, radius);
        bbox = bounds3(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    Sphere(const point3& center1, const point3& center2, const double radius, const shared_ptr<Material> &mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = vec3(radius, radius, radius);
        const bounds3 box1(center.at(0) - rvec, center.at(0) + rvec);
        const bounds3 box2(center.at(1) - rvec, center.at(1) + rvec);
        bbox = bounds3(box1, box2);
    }

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

    std::optional<shape_intersection> intersect(const ray& r, const interval ray_t, shape_intersection& rec) const override {
        point3 current_center = center.at(r.time());
        vec3 oc = current_center - r.o();
        const auto a = r.d().length_squared();
        const auto h = dot(r.d(), oc);
        const auto c = oc.length_squared() - radius*radius;

        const auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return {};

        const auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return {};
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        const vec3 outward_normal = (rec.p - current_center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;

        return {rec};
    }

    [[nodiscard]] double pdf(const point3& origin, const vec3& direction) const override {
        // This method only works for stationary spheres.
        shape_intersection rec;
        if (!this->intersect(ray(origin, direction), interval(0.001, infinity), rec))
            return 0;

        const auto dist_squared = (center.at(0) - origin).length_squared();
        const auto cos_theta_max = std::sqrt(1 - radius*radius/dist_squared);
        const auto solid_angle = 2*pi*(1-cos_theta_max);

        return  1 / solid_angle;
    }

    [[nodiscard]] vec3 random(const point3& origin, const std::shared_ptr<sampler>& sampler) const override {
        const vec3 direction = center.at(0) - origin;
        auto distance_squared = direction.length_squared();
        const orthonormal_base uvw(direction);
        return uvw.transform(random_to_sphere(sampler->gen_2d(), radius, distance_squared));
    }

private:
    ray center;
    double radius;
    shared_ptr<Material> mat;
    bounds3 bbox;

private:
    static void get_sphere_uv(const point3& p, double& u, double& v) {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

        const auto theta = std::acos(-p.y());
        const auto phi = std::atan2(-p.z(), p.x()) + pi;

        u = phi / (2*pi);
        v = theta / pi;
    }
};

#endif
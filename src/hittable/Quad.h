//
// Created by polup on 01/01/2026.
//

#ifndef QUAD_H
#define QUAD_H
#include <memory>

#include "HittableList.h"


class Quad final : public shape {
public:
    Quad(const point3& Q, const vec3& u, const vec3& v, const std::shared_ptr<Material> &mat)
      : Q(Q), u(u), v(v), mat(mat)
    {
        const auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n,n);

        area = n.length();

        bounding_box();
    }

    void bounding_box() {
        // Compute the bounding box of all four vertices.
        const auto bbox_diagonal1 = bounds3(Q, Q + u + v);
        const auto bbox_diagonal2 = bounds3(Q + u, Q + v);
        bbox = bounds3(bbox_diagonal1, bbox_diagonal2);
    }

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

    bool intersect(const ray& r, const interval ray_t, HitRecord& rec) const override {
        const auto denom = dot(normal, r.d());

        // No hit if the ray is parallel to the plane.
        if (std::fabs(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        const auto t = (D - dot(normal, r.o())) / denom;
        if (!ray_t.contains(t))
            return false;

        const auto intersection = r.at(t);
        const vec3 planar_hitpt_vector = intersection - Q;
        const auto alpha = dot(w, cross(planar_hitpt_vector, v));
        const auto beta = dot(w, cross(u, planar_hitpt_vector));

        if (!isInterior(alpha, beta, rec))
            return false;

        // Ray hits the 2D shape; set the rest of the hit record and return true.

        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal(r, normal);

        return true;
    }

    static bool isInterior(const double a, const double b, HitRecord& rec) {
        const auto unit_interval = interval(0, 1);
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.

        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

    [[nodiscard]] double pdf(const point3& origin, const vec3& direction) const override {
        HitRecord rec;
        if (!this->intersect(ray(origin, direction), interval(0.001, infinity), rec))
            return 0;

        const auto distance_squared = rec.t * rec.t * direction.length_squared();
        const auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

        return distance_squared / (cosine * area);
    }

    [[nodiscard]] vec3 random(const point3& origin) const override {
        const auto p = Q + (random_double() * u) + (random_double() * v);
        return p - origin;
    }

private:
    point3 Q;
    vec3 u, v;
    vec3 w;
    shared_ptr<Material> mat;
    bounds3 bbox;
    vec3 normal;
    double D;
    double area;
};

inline shared_ptr<HittableList> box(const point3& a, const point3& b, const shared_ptr<Material>& mat)
{
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<HittableList>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    const auto min = point3(std::fmin(a.x(),b.x()), std::fmin(a.y(),b.y()), std::fmin(a.z(),b.z()));
    const auto max = point3(std::fmax(a.x(),b.x()), std::fmax(a.y(),b.y()), std::fmax(a.z(),b.z()));

    auto dx = vec3(max.x() - min.x(), 0, 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(make_shared<Quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(make_shared<Quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(make_shared<Quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(make_shared<Quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(make_shared<Quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(make_shared<Quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}

#endif //QUAD_H

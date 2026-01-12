//
// Created by polup on 01/01/2026.
//

#ifndef QUAD_H
#define QUAD_H
#include <memory>

#include "HittableList.h"


class Quad final : public Hittable {
public:
    Quad(const Point3& Q, const Vec3& u, const Vec3& v, const std::shared_ptr<Material> &mat)
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
        const auto bbox_diagonal1 = AABB(Q, Q + u + v);
        const auto bbox_diagonal2 = AABB(Q + u, Q + v);
        bbox = AABB(bbox_diagonal1, bbox_diagonal2);
    }

    [[nodiscard]] AABB boundingBox() const override { return bbox; }

    bool hit(const Ray& r, const Interval ray_t, HitRecord& rec) const override {
        const auto denom = dot(normal, r.direction());

        // No hit if the ray is parallel to the plane.
        if (std::fabs(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        const auto t = (D - dot(normal, r.origin())) / denom;
        if (!ray_t.contains(t))
            return false;

        const auto intersection = r.at(t);
        const Vec3 planar_hitpt_vector = intersection - Q;
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
        const auto unit_interval = Interval(0, 1);
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.

        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

    [[nodiscard]] double pdfValue(const Point3& origin, const Vec3& direction) const override {
        HitRecord rec;
        if (!this->hit(Ray(origin, direction), Interval(0.001, infinity), rec))
            return 0;

        const auto distance_squared = rec.t * rec.t * direction.length_squared();
        const auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

        return distance_squared / (cosine * area);
    }

    [[nodiscard]] Vec3 random(const Point3& origin) const override {
        const auto p = Q + (random_double() * u) + (random_double() * v);
        return p - origin;
    }

private:
    Point3 Q;
    Vec3 u, v;
    Vec3 w;
    shared_ptr<Material> mat;
    AABB bbox;
    Vec3 normal;
    double D;
    double area;
};

inline shared_ptr<HittableList> box(const Point3& a, const Point3& b, const shared_ptr<Material>& mat)
{
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<HittableList>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    const auto min = Point3(std::fmin(a.x(),b.x()), std::fmin(a.y(),b.y()), std::fmin(a.z(),b.z()));
    const auto max = Point3(std::fmax(a.x(),b.x()), std::fmax(a.y(),b.y()), std::fmax(a.z(),b.z()));

    auto dx = Vec3(max.x() - min.x(), 0, 0);
    auto dy = Vec3(0, max.y() - min.y(), 0);
    auto dz = Vec3(0, 0, max.z() - min.z());

    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(make_shared<Quad>(Point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(make_shared<Quad>(Point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(make_shared<Quad>(Point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}

#endif //QUAD_H

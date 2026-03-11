//
// Created by polup on 20/01/2026.
//

#include "shapes.h"

quad::quad(const point3& Q, const vec3& u, const vec3& v, const std::shared_ptr<material>& mat): Q(Q), u(u), v(v), mat(mat)
{
    const auto n = cross(u, v);
    normal = unit_vector(n);
    D = dot(normal, Q);
    w = n / dot(n,n);

    area = n.length();

    bounding_box();
}

void quad::bounding_box()
{
    // Compute the bounding box of all four vertices.
    const auto bbox_diagonal1 = bounds3(Q, Q + u + v);
    const auto bbox_diagonal2 = bounds3(Q + u, Q + v);
    bbox = bounds3(bbox_diagonal1, bbox_diagonal2);
}

std::optional<shape_intersection> quad::intersect(const ray& r, const interval ray_t) const
{
    const auto denom = dot(normal, r.d());

    if (std::fabs(denom) < 1e-8)
        return {};

    const auto t = (D - dot(normal, r.o())) / denom;
    if (!ray_t.contains(t))
        return {};

    const auto intersection = r.at(t);
    const vec3 planar_hitpt_vector = intersection - Q;
    const auto alpha = dot(w, cross(planar_hitpt_vector, v));
    const auto beta = dot(w, cross(u, planar_hitpt_vector));

    shape_intersection rec;
    if (!is_interior(alpha, beta, rec))
        return {};

    rec.t = t;
    rec.p = intersection;
    rec.mat = mat;
    rec.set_face_normal(r, normal);

    return rec;
}

bool quad::is_interior(const double a, const double b, shape_intersection& rec)
{
    const auto unit_interval = interval(0, 1);
    // Given the hit point in plane coordinates, return false if it is outside the
    // primitive, otherwise set the hit record UV coordinates and return true.

    if (!unit_interval.contains(a) || !unit_interval.contains(b))
        return false;

    rec.u = a;
    rec.v = b;
    return true;
}

double quad::pdf(const point3& origin, const vec3& direction) const
{
    // Use a temporary intersection just to get t and normal for PDF calc if needed,
    // but since we just returned from intersect check, we need the data.
    // We have to call intersect again to get the data in this design.
    const auto rec = this->intersect(ray(origin, direction), interval(0.001, infinity));
    if (!rec) return 0;

    const auto distance_squared = rec->t * rec->t * direction.length_squared();
    const auto cosine = std::fabs(dot(direction, rec->normal) / direction.length());

    return distance_squared / (cosine * area);
}

RotateY::RotateY(const shared_ptr<shape>& object, const double angle): object(object)
{
    const auto radians = degrees_to_radians(angle);
    sin_theta = std::sin(radians);
    cos_theta = std::cos(radians);
    bbox = object->bounds();

    point3 min(infinity,  infinity,  infinity);
    point3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                const auto x = i*bbox.x.max + (1-i)*bbox.x.min;
                const auto y = j*bbox.y.max + (1-j)*bbox.y.min;
                const auto z = k*bbox.z.max + (1-k)*bbox.z.min;

                const auto newx =  cos_theta*x + sin_theta*z;
                const auto newz = -sin_theta*x + cos_theta*z;

                vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = std::fmin(min[c], tester[c]);
                    max[c] = std::fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = bounds3(min, max);
}

std::optional<shape_intersection> RotateY::intersect(const ray& r, const interval ray_t) const
{
    const auto origin = point3(
        (cos_theta * r.o().x()) - (sin_theta * r.o().z()),
        r.o().y(),
        (sin_theta * r.o().x()) + (cos_theta * r.o().z())
    );

    const auto direction = vec3(
        (cos_theta * r.d().x()) - (sin_theta * r.d().z()),
        r.d().y(),
        (sin_theta * r.d().x()) + (cos_theta * r.d().z())
    );

    ray rotated_r(origin, direction, r.time());

    auto result = object->intersect(rotated_r, ray_t);
    if (!result) return {};

    result->p = point3(
        (cos_theta * result->p.x()) + (sin_theta * result->p.z()),
        result->p.y(),
        (-sin_theta * result->p.x()) + (cos_theta * result->p.z())
    );

    result->normal = vec3(
        (cos_theta * result->normal.x()) + (sin_theta * result->normal.z()),
        result->normal.y(),
        (-sin_theta * result->normal.x()) + (cos_theta * result->normal.z())
    );

    return result;
}

std::optional<shape_intersection> sphere::intersect(const ray& r, interval ray_t) const
{
    const point3 current_center = center.at(r.time());
    const vec3 oc = current_center - r.o();
    const auto a = r.d().length_squared();
    const auto h = dot(r.d(), oc);
    const auto c = oc.length_squared() - radius*radius;

    const auto discriminant = h*h - a*c;
    if (discriminant < 0)
        return {};

    const auto sqrtd = std::sqrt(discriminant);

    auto root = (h - sqrtd) / a;
    if (!ray_t.surrounds(root)) {
        root = (h + sqrtd) / a;
        if (!ray_t.surrounds(root))
            return {};
    }

    shape_intersection rec;
    rec.t = root;
    rec.p = r.at(rec.t);
    const vec3 outward_normal = (rec.p - current_center) / radius;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat = mat;

    return rec;
}

double sphere::pdf(const point3& origin, const vec3& direction) const
{
    // This method only works for stationary spheres.
    if (!this->intersect(ray(origin, direction), interval(0.001, infinity)))
        return 0;

    const auto dist_squared = (center.at(0) - origin).length_squared();
    const auto cos_theta_max = std::sqrt(1 - radius*radius/dist_squared);
    const auto solid_angle = 2*pi*(1-cos_theta_max);

    return  1 / solid_angle;
}

vec3 sphere::random(const point3& origin, const std::shared_ptr<sampler>& sampler) const
{
    const vec3 direction = center.at(0) - origin;
    const auto distance_squared = direction.length_squared();
    const frame uvw(direction);
    return uvw.transform(random_to_sphere(sampler->gen_2d(), radius, distance_squared));
}

void sphere::get_sphere_uv(const point3& p, double& u, double& v)
{
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

std::optional<shape_intersection> translate::intersect(const ray& r, const interval ray_t) const
{
    const ray offset_r(r.o() - offset, r.d(), r.time());

    auto result = object->intersect(offset_r, ray_t);
    if (!result) return {};

    result->p += offset;
    return result;
}

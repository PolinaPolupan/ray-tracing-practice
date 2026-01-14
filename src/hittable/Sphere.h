#ifndef SPHERE_H
#define SPHERE_H


class Sphere final : public shape {
public:
    // Stationary Sphere
    Sphere(const point3d& static_center, double radius, const shared_ptr<Material> &mat)
      : center(static_center, vec3d(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = vec3d(radius, radius, radius);
        bbox = bounds3d(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    Sphere(const point3d& center1, const point3d& center2, const double radius, const shared_ptr<Material> &mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = vec3d(radius, radius, radius);
        const bounds3d box1(center.at(0) - rvec, center.at(0) + rvec);
        const bounds3d box2(center.at(1) - rvec, center.at(1) + rvec);
        bbox = bounds3d(box1, box2);
    }

    [[nodiscard]] bounds3d bounds() const override { return bbox; }

    bool intersect(const ray& r, const interval ray_t, HitRecord& rec) const override {
        point3d current_center = center.at(r.time());
        vec3d oc = current_center - r.o();
        const auto a = r.d().length_squared();
        const auto h = dot(r.d(), oc);
        const auto c = oc.length_squared() - radius*radius;

        const auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        const auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        const vec3d outward_normal = (rec.p - current_center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;

        return true;
    }

    [[nodiscard]] double pdf(const point3d& origin, const vec3d& direction) const override {
        // This method only works for stationary spheres.
        HitRecord rec;
        if (!this->intersect(ray(origin, direction), interval(0.001, infinity), rec))
            return 0;

        const auto dist_squared = (center.at(0) - origin).length_squared();
        const auto cos_theta_max = std::sqrt(1 - radius*radius/dist_squared);
        const auto solid_angle = 2*pi*(1-cos_theta_max);

        return  1 / solid_angle;
    }

    [[nodiscard]] vec3d random(const point3d& origin) const override {
        const vec3d direction = center.at(0) - origin;
        auto distance_squared = direction.length_squared();
        const orthonormal_base uvw(direction);
        return uvw.transform(random_to_sphere(radius, distance_squared));
    }

private:
    ray center;
    double radius;
    shared_ptr<Material> mat;
    bounds3d bbox;

private:
    static void get_sphere_uv(const point3d& p, double& u, double& v) {
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

    static vec3d random_to_sphere(const double radius, const double distance_squared) {
        const auto r1 = random_double();
        const auto r2 = random_double();
        auto z = 1 + r2*(std::sqrt(1-radius*radius/distance_squared) - 1);

        const auto phi = 2*pi*r1;
        auto x = std::cos(phi) * std::sqrt(1-z*z);
        auto y = std::sin(phi) * std::sqrt(1-z*z);

        return {x, y, z};
    }
};

#endif
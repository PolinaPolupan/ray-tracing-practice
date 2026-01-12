#ifndef SPHERE_H
#define SPHERE_H


class Sphere final : public Hittable {
public:
    // Stationary Sphere
    Sphere(const Point3& static_center, double radius, const shared_ptr<Material> &mat)
      : center(static_center, Vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = Vec3(radius, radius, radius);
        bbox = AABB(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    Sphere(const Point3& center1, const Point3& center2, const double radius, const shared_ptr<Material> &mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {
        const auto rvec = Vec3(radius, radius, radius);
        const AABB box1(center.at(0) - rvec, center.at(0) + rvec);
        const AABB box2(center.at(1) - rvec, center.at(1) + rvec);
        bbox = AABB(box1, box2);
    }

    [[nodiscard]] AABB boundingBox() const override { return bbox; }

    bool hit(const Ray& r, const Interval ray_t, HitRecord& rec) const override {
        Point3 current_center = center.at(r.time());
        Vec3 oc = current_center - r.origin();
        const auto a = r.direction().length_squared();
        const auto h = dot(r.direction(), oc);
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
        const Vec3 outward_normal = (rec.p - current_center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;

        return true;
    }

    [[nodiscard]] double pdfValue(const Point3& origin, const Vec3& direction) const override {
        // This method only works for stationary spheres.
        HitRecord rec;
        if (!this->hit(Ray(origin, direction), Interval(0.001, infinity), rec))
            return 0;

        const auto dist_squared = (center.at(0) - origin).length_squared();
        const auto cos_theta_max = std::sqrt(1 - radius*radius/dist_squared);
        const auto solid_angle = 2*pi*(1-cos_theta_max);

        return  1 / solid_angle;
    }

    [[nodiscard]] Vec3 random(const Point3& origin) const override {
        const Vec3 direction = center.at(0) - origin;
        auto distance_squared = direction.length_squared();
        const ONB uvw(direction);
        return uvw.transform(random_to_sphere(radius, distance_squared));
    }

private:
    Ray center;
    double radius;
    shared_ptr<Material> mat;
    AABB bbox;

private:
    static void get_sphere_uv(const Point3& p, double& u, double& v) {
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

    static Vec3 random_to_sphere(const double radius, const double distance_squared) {
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
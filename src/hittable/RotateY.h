//
// Created by polup on 03/01/2026.
//

#ifndef ROTATEY_H
#define ROTATEY_H


class RotateY final : public shape {
public:
    RotateY(const shared_ptr<shape>& object, double angle) : object(object) {
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

    [[nodiscard]] std::optional<shape_intersection> intersect(const ray& r, const interval ray_t) const override {
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

    [[nodiscard]] bounds3 bounds() const override { return bbox; }

private:
    shared_ptr<shape> object;
    double sin_theta;
    double cos_theta;
    bounds3 bbox;
};

#endif //ROTATEY_H

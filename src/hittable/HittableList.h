//
// Created by polup on 03/01/2026.
//

#ifndef HITTABLELIST_H
#define HITTABLELIST_H


class HittableList final : public Hittable {
public:
    std::vector<shared_ptr<Hittable>> objects;

    HittableList() = default;
    explicit HittableList(const shared_ptr<Hittable>& object) { add(object); }

    void clear() { objects.clear(); }

    void add(const shared_ptr<Hittable>& object) {
        objects.push_back(object);
        bbox = AABB(bbox, object->boundingBox());
    }

    bool hit(const Ray& r, const Interval ray_t, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, Interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    [[nodiscard]] AABB boundingBox() const override { return bbox; }

    [[nodiscard]] double pdfValue(const Point3& origin, const Vec3& direction) const override {
        const auto weight = 1.0 / objects.size();
        auto sum = 0.0;

        for (const auto& object : objects)
            sum += weight * object->pdfValue(origin, direction);

        return sum;
    }

    [[nodiscard]] Vec3 random(const Point3& origin) const override {
        const auto int_size = static_cast<int>(objects.size());
        return objects[random_int(0, int_size-1)]->random(origin);
    }

private:
    AABB bbox;
};


#endif //HITTABLELIST_H

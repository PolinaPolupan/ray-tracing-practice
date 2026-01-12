//
// Created by polup on 06/01/2026.
//

#ifndef HITTABLEPDF_H
#define HITTABLEPDF_H
#include "PDF.h"
#include "../hittable/Hittable.h"

class HittablePdf: public PDF {
public:
    HittablePdf(const Hittable& objects, const Point3& origin)
      : objects(objects), origin(origin)
    {}

    [[nodiscard]] double value(const Vec3& direction) const override {
        return objects.pdfValue(origin, direction);
    }

    [[nodiscard]] Vec3 generate() const override {
        return objects.random(origin);
    }

private:
    const Hittable& objects;
    Point3 origin;

};



#endif //HITTABLEPDF_H

//
// Created by polup on 25/12/2025.
//

#ifndef TEXTURE_H
#define TEXTURE_H



class Texture {
public:
    virtual ~Texture() = default;

    virtual Color value(double u, double v, const Point3& p) const = 0;
};



#endif //TEXTURE_H

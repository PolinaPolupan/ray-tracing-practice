//
// Created by polup on 25/12/2025.
//

#ifndef TEXTURE_H
#define TEXTURE_H



class Texture {
public:
    virtual ~Texture() = default;

    [[nodiscard]] virtual color value(double u, double v, const point3d& p) const = 0;
};



#endif //TEXTURE_H

//
// Created by polup on 11/01/2026.
//

#ifndef FILM_H
#define FILM_H

#include "math.h"


class film {
public:
    double linear_to_gamma(double linear_component);

    void write_color(std::ostream& out, const color& pixel_color);
};



#endif //FILM_H

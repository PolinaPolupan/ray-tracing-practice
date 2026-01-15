//
// Created by polup on 13/01/2026.
//

#include "sampling.h"

vec3 sample_square_stratified(const point2 u, const int s_i, const int s_j, const double recip_sqrt_spp) {
    // Returns the vector to a random point in the square sub-pixel specified by grid
    // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

    auto px = ((s_i + u.x) * recip_sqrt_spp) - 0.5;
    auto py = ((s_j + u.y) * recip_sqrt_spp) - 0.5;

    return {px, py, 0};
}

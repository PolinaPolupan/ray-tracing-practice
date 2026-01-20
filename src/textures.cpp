//
// Created by polup on 20/01/2026.
//

#include "textures.h"

double perlin::noise(const point3& p) const
{
    const auto u = p.x() - std::floor(p.x());
    const auto v = p.y() - std::floor(p.y());
    const auto w = p.z() - std::floor(p.z());

    const auto i = static_cast<int>(std::floor(p.x()));
    const auto j = static_cast<int>(std::floor(p.y()));
    const auto k = static_cast<int>(std::floor(p.z()));
    vec3 c[2][2][2];

    for (int di=0; di < 2; di++)
        for (int dj=0; dj < 2; dj++)
            for (int dk=0; dk < 2; dk++)
                c[di][dj][dk] = randvec[
                    perm_x[(i+di) & 255] ^
                    perm_y[(j+dj) & 255] ^
                    perm_z[(k+dk) & 255]
                ];

    return perlin_interp(c, u, v, w);
}

double perlin::turb(const point3& p, const int depth) const
{
    auto accum = 0.0;
    auto temp_p = p;
    auto weight = 1.0;

    for (int i = 0; i < depth; i++) {
        accum += weight * noise(temp_p);
        weight *= 0.5;
        temp_p *= 2;
    }

    return std::fabs(accum);
}

double perlin::perlin_interp(const vec3 c[2][2][2], const double u, const double v, const double w)
{
    const auto uu = u*u*(3-2*u);
    const auto vv = v*v*(3-2*v);
    const auto ww = w*w*(3-2*w);
    auto accum = 0.0;

    for (int i=0; i < 2; i++)
        for (int j=0; j < 2; j++)
            for (int k=0; k < 2; k++) {
                vec3 weight_v(u-i, v-j, w-k);
                accum += (i*uu + (1-i)*(1-uu))
                       * (j*vv + (1-j)*(1-vv))
                       * (k*ww + (1-k)*(1-ww))
                       * dot(c[i][j][k], weight_v);
            }

    return accum;
}

color image::value(double u, double v, const point3& p) const
{
    // If we have no texture data, then return solid cyan as a debugging aid.
    if (image_.height() <= 0) return {0,1,1};

    // Clamp input texture coordinates to [0,1] x [1,0]
    u = interval(0,1).clamp(u);
    v = 1.0 - interval(0,1).clamp(v);  // Flip V to image coordinates

    const auto i = static_cast<int>(u * image_.width());
    const auto j = static_cast<int>(v * image_.height());
    const auto pixel = image_.pixelData(i,j);

    constexpr auto colorScale = 1.0 / 255.0;
    return {colorScale*pixel[0], colorScale*pixel[1], colorScale*pixel[2]};
}

color checker::value(const double u, const double v, const point3& p) const
{
    const auto xInteger = static_cast<int>(std::floor(inv_scale_ * p.x()));
    const auto yInteger = static_cast<int>(std::floor(inv_scale_ * p.y()));
    const auto zInteger = static_cast<int>(std::floor(inv_scale_ * p.z()));

    const bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    return isEven ? even_->value(u, v, p) : odd_->value(u, v, p);
}

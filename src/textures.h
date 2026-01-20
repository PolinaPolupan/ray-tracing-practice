#ifndef RAY_TRACING_IN_ONE_WEEK_TEXTURES_H
#define RAY_TRACING_IN_ONE_WEEK_TEXTURES_H
#include "math.h"
#include "RtwImage.h"

class perlin {
public:
    perlin() {
        for (auto & i : randvec) {
            i = unit_vector({random_double(-1.0, 1.0), random_double(-1.0, 1.0), random_double(-1.0, 1.0)});
        }

        perlin_generate_perm(perm_x);
        perlin_generate_perm(perm_y);
        perlin_generate_perm(perm_z);
    }

    [[nodiscard]] double noise(const point3& p) const;

    [[nodiscard]] double turb(const point3& p, int depth) const;

private:
    static constexpr int point_count = 256;
    vec3 randvec[point_count];
    int perm_x[point_count]{};
    int perm_y[point_count]{};
    int perm_z[point_count]{};

    static void perlin_generate_perm(int* p) {
        for (int i = 0; i < point_count; i++)
            p[i] = i;

        permute(p, point_count);
    }

    static void permute(int* p, int n) {
        for (int i = n-1; i > 0; i--) {
            const int target = random_int(0, i);
            const int tmp = p[i];
            p[i] = p[target];
            p[target] = tmp;
        }
    }

    static double perlin_interp(const vec3 c[2][2][2], double u, double v, double w);

    static int random_int(const int min, const int max) {
        thread_local std::mt19937 generator{std::random_device{}()};
        std::uniform_int_distribution distribution(min, max);
        return distribution(generator);
    }

    static double random_double(const double min = 0.0, const double max = 1.0) {
        thread_local std::mt19937 gen{1337}; // fixed seed = deterministic
        std::uniform_real_distribution dist(min, max);
        return dist(gen);
    }
};

class texture {
public:
    virtual ~texture() = default;

    [[nodiscard]] virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color final : public texture {
public:
    explicit solid_color(const color& albedo) : albedo_(albedo) {}

    solid_color(const double red, const double green, const double blue) : solid_color(color(red,green,blue)) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override {
        return albedo_;
    }

private:
    color albedo_;
};

class noise final : public texture {
public:
    explicit noise(const double scale) : scale_(scale) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override {
        return color(.5, .5, .5) * (1 + std::sin(scale_ * p.z() + 10 * noise_.turb(p, 7)));
    }

private:
    perlin noise_{};
    double scale_;
};

class image final : public texture {
public:
    explicit image(const char* filename) : image_(filename) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override;

private:
    RtwImage image_;
};

class checker final : public texture {
public:
    checker(const double scale, const std::shared_ptr<texture> &even, const std::shared_ptr<texture> &odd)
      : inv_scale_(1.0 / scale), even_(even), odd_(odd) {}

    checker(const double scale, const color& c1, const color& c2)
      : checker(scale, std::make_shared<solid_color>(c1), std::make_shared<solid_color>(c2)) {}

    [[nodiscard]] color value(double u, double v, const point3& p) const override;

private:
    double inv_scale_;
    shared_ptr<texture> even_;
    shared_ptr<texture> odd_;
};


#endif //RAY_TRACING_IN_ONE_WEEK_TEXTURES_H
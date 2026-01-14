//
// Created by polup on 06/01/2026.
//

#ifndef MIXTUREPDF_H
#define MIXTUREPDF_H


class MixturePdf final : public PDF {
public:
    MixturePdf(const shared_ptr<PDF> &p0, const shared_ptr<PDF> &p1) {
        p[0] = p0;
        p[1] = p1;
    }

    [[nodiscard]] double value(const vec3d& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    [[nodiscard]] vec3d generate() const override {
        if (random_double() < 0.5)
            return p[0]->generate();
        return p[1]->generate();
    }

private:
    shared_ptr<PDF> p[2];
};



#endif //MIXTUREPDF_H

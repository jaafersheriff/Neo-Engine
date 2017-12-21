#include "Atmosphere.hpp"

Atmosphere::Atmosphere(Mesh *m, Texture *t1, Texture *t2) {
    this->mesh = m;
    this->colorTexture = t1;
    this->glowTexture = t2;
}

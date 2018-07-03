#include "Atmosphere.hpp"

Atmosphere::Atmosphere(Mesh *m, Texture *t1, Texture *t2, float size) {
    this->mesh = m;
    this->colorTexture = t1;
    this->glowTexture = t2;
    this->size = size;
}

#pragma once
#ifndef _ATMOSPHERE_HPP_
#define _ATMOSPHERE_HPP_

#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

class Atmosphere {
    public:
        Atmosphere(Mesh *, Texture *, Texture *, float);
        float size;
        Mesh *mesh;
        Texture *colorTexture;
        Texture *glowTexture;
};

#endif

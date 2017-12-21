#pragma once
#ifndef _ATMOSPHERE_HPP_
#define _ATMOSPHERE_HPP_

#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

#define SIZE 1000.f 

class Atmosphere {
    public:
        Atmosphere(Mesh *, Texture *, Texture *);
        Mesh *mesh;
        Texture *colorTexture;
        Texture *glowTexture;
};

#endif

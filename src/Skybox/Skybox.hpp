#pragma once
#ifndef _SKYBOX_HPP_
#define _SKYBOX_HPP_

#include "Model/Mesh.hpp"
#include "Skybox/CubeTexture.hpp"

#include <vector>

#define ROTATE_SPEED 5.f

class Skybox {
    public: 
        Skybox() {}
        Skybox(Mesh *, CubeTexture *);
        void update(const float);
        
        Mesh *mesh;
        CubeTexture *cubeTexture;

        float rotation = 0.f;

        
};

#endif

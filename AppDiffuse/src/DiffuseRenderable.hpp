#pragma once 

#include "Component/RenderableComponent/RenderableComponent.hpp"

#include "Model/Texture.hpp"

using namespace neo;

class DiffuseRenderable : public RenderableComponent {

    public:
        DiffuseRenderable(GameObject &go, Mesh *mesh,Material *mat, Texture *tex) :
            RenderableComponent(go, mesh, mat),
            texture(tex)
        {}

        Texture & getTexture() { return *texture; }

    private:

        Texture *texture;
        
};
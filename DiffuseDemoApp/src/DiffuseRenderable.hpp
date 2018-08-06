#pragma once 

#include "Component/RenderableComponent/RenderableComponent.hpp"

#include "Model/Texture.hpp"
#include "Model/Material.hpp"

using namespace neo;

class DiffuseRenderable : public RenderableComponent {

    public:
        DiffuseRenderable(GameObject &go, Mesh *mesh, Texture *tex, Material *mat) :
            RenderableComponent(go, mesh),
            texture(tex),
            material(mat)
        {}

        Texture & getTexture() { return *texture; }
        Material & getMaterial() { return *material; }

    private:

        Texture *texture;
        Material *material;
        
};
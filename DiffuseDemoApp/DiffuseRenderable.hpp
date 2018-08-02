#pragma once 

#include "Component/RenderableComponent/RenderableComponent.hpp"

#include "Model/ModelTexture.hpp"

using namespace neo;

class DiffuseRenderable : public RenderableComponent {

    public:
        DiffuseRenderable(GameObject &go, Mesh *m, ModelTexture t) :
            RenderableComponent(go, m),
            modelTexture(t)
        {}

        ModelTexture & getModelTexture() { return modelTexture; }

    private:

        ModelTexture modelTexture;
        
};
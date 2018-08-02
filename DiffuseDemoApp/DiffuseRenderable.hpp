#pragma once 

#include "Component/RenderableComponent/RenderableComponent.hpp"

#include "Model/ModelTexture.hpp"

using namespace neo;

class DiffuseRenderable : public RenderableComponent {

    public:
        DiffuseRenderable(GameObject &go, Mesh *m, ModelTexture t) :
            RenderableComponent(go, m),
            texture(t)
        {}

        const ModelTexture & getTexture() const { return texture; }
        void replaceTexture(ModelTexture & t) {
            //this->texture = t;
        }

        ModelTexture texture;
        
};
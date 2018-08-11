#pragma once

#include "RenderableComponent.hpp"

#include "Model/Texture.hpp"

namespace neo {

    class TexturedRenderable : public RenderableComponent {

        public:

            TexturedRenderable(GameObject &go, Mesh *mesh, Material *mat, Texture *tex) :
                RenderableComponent(go, mesh, mat),
                texture(tex)
            {}
                
            const Texture & getTexture() const { return *texture; }

         private:
             Texture * texture;

    };
}
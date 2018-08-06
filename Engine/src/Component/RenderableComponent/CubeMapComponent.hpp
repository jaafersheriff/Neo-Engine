#pragma once

#include "Model/Texture.hpp"

#include "RenderableComponent.hpp"

#include "Loader/Loader.hpp"

namespace neo {

    class CubeMapComponent : public RenderableComponent {

        public:

            CubeMapComponent(GameObject &go, Texture *tex) :
                RenderableComponent(go, Loader::getMesh("cube")),
                texture(tex)
            {}
                
            const Texture *getTexture() const { return texture; }

         private:
             Texture * texture;

    };
}
#pragma once

#include "RenderableComponent.hpp"

namespace neo {

    class CubeMapComponent : public RenderableComponent {

        public:

            CubeMapComponent(GameObject &go, CubeTexture *tex) :
                RenderableComponent(go, Loader::getMesh("cube")),
                texture(tex)
            {}
                
            const CubeTexture *getTexture() const { return texture; }

         private:
             CubeTexture * texture;

    };
}
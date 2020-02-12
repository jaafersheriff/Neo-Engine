#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Texture.hpp"

// TODO : template class..
namespace neo {

    class DiffuseMapComponent : public Component {

        public:
            DiffuseMapComponent(GameObject *go, const Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            const Texture& mTexture;
    };

    class SpecularMapComponent : public Component {

        public:
            SpecularMapComponent(GameObject *go, const Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            const Texture& mTexture;

    };

    class NormalMapComponent : public Component {

        public:
            NormalMapComponent(GameObject *go, const Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            const Texture& mTexture;

    };

    class CubeMapComponent : public Component {

        public:
            CubeMapComponent(GameObject *go, Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            const Texture& mTexture;

    };
}

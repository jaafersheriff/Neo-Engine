#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Texture.hpp"

// TODO : template class..
namespace neo {
 class AlphaMapComponent : public Component {

        public:
            AlphaMapComponent(GameObject *go, Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            Texture& mTexture;
    };


    class DisplacementMapComponent : public Component {

        public:
            DisplacementMapComponent(GameObject *go, Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            Texture& mTexture;
    };


    class AmbientMapComponent : public Component {

        public:
            AmbientMapComponent(GameObject *go, Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            Texture& mTexture;
    };


    class DiffuseMapComponent : public Component {

        public:
            DiffuseMapComponent(GameObject *go, Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            Texture& mTexture;
    };

    class SpecularMapComponent : public Component {

        public:
            SpecularMapComponent(GameObject *go, Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            Texture& mTexture;

    };

    class NormalMapComponent : public Component {

        public:
            NormalMapComponent(GameObject *go, Texture& tex) :
                Component(go),
                mTexture(tex)
            {}

            Texture& mTexture;

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

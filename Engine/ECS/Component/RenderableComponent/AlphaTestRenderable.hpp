#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        struct AlphaTestRenderable : public Component {
            Texture* mDiffuseMap;
            AlphaTestRenderable(Texture* texture) :
                mDiffuseMap(texture)
            {}

            virtual std::string getName() const override { return "AlphaTestRenderable"; }

        };
    }
}
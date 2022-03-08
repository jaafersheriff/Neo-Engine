#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        struct ShadowCasterRenderable : public Component {
            const Texture& mAlphaMap;

            ShadowCasterRenderable(const Texture& texture) :
                mAlphaMap(texture)
            {}

            virtual std::string getName() override {
                return "ShadowCasterRenderable";
            }

        };
    }
}
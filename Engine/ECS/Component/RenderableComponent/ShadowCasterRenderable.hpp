#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class Texture;

    namespace renderable {

        struct ShadowCasterRenderable : public Component {
            Texture* mAlphaMap;

            ShadowCasterRenderable(Texture* texture) :
                mAlphaMap(texture)
            {}

            virtual std::string getName() const override {
                return "ShadowCasterRenderable";
            }

        };
    }
}
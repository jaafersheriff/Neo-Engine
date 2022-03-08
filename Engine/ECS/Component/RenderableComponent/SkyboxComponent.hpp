#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
    namespace renderable {
        struct SkyboxComponent : public Component {
            const Texture& mCubeMap;

            SkyboxComponent(const Texture& cubeMap) :
                mCubeMap(cubeMap)
            {}

            virtual std::string getName() override {
                return "SkyboxComponent";
            }
        };
    }
}
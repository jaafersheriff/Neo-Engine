#pragma once

#include "ECS/Component/Component.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include <glm/glm.hpp>

using namespace neo;

namespace Froxels {
    struct MockCameraComponent : public Component {
        MockCameraComponent()
        {}
        virtual std::string getName() const override { return "MockCameraComponent"; }
        virtual void imGuiEditor() override {

        }
    };
}

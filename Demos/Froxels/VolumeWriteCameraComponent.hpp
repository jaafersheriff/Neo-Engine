#pragma once

#include "ECS/Component/Component.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include <glm/glm.hpp>

using namespace neo;

namespace Froxels {
    struct VolumeWriteCameraComponent : public Component {
        VolumeWriteCameraComponent()
        {}
        virtual std::string getName() const override { return "VolumeWriteCameraComponent"; }
        virtual void imGuiEditor() override {

        }
    };
}

#pragma once

#include "ECS/Component/Component.hpp"
#include "Renderer/GLObjects/Texture.hpp"

#include <glm/glm.hpp>

using namespace neo;

struct VolumeComponent : public Component {
    VolumeComponent(Texture* tex)
        : mTexture(tex)
    {}
    virtual std::string getName() const override { return "VolumeComponent"; }
    virtual void imGuiEditor() override {

    }

    Texture* mTexture;
};

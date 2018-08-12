#pragma once

#include "Component/RenderableComponent/RenderableModel.hpp"

#include "Loader/Loader.hpp"

using namespace neo;

class SkyboxComponent : public TexturedRenderable {
    public:
        SkyboxComponent(GameObject *go, Texture *tex) :
            TexturedRenderable(go, Loader::getMesh("cube"), tex)
        {}
};
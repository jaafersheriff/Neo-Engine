#pragma once

#include "Component/RenderableComponent/TexturedRenderable.hpp"

#include "Loader/Loader.hpp"

using namespace neo;

class SkyboxComponent : public TexturedRenderable {
public:
    SkyboxComponent(GameObject &go, Texture *tex) :
        TexturedRenderable(go, Loader::getMesh("cube"), nullptr, tex)
    {}
};
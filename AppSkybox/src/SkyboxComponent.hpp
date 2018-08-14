#pragma once

#include "Component/ModelComponent/RenderableComponent.hpp"

#include "Loader/Loader.hpp"

using namespace neo;

class SkyboxComponent : public RenderableComponent {
    public:
        SkyboxComponent(GameObject *go) :
            RenderableComponent(go, Loader::getMesh("cube"))
        {}
};
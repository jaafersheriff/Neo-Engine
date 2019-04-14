#pragma once

#include "Loader/Loader.hpp"

using namespace neo;

class SkyboxComponent : public Component {
    public:
        SkyboxComponent(GameObject *go) :
            Component(go) 
        {}
};
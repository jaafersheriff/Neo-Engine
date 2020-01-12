#pragma once

#include "Component/Component.hpp"

namespace neo {

    class FrustumFitSourceComponent : public Component {
    public:
        FrustumFitSourceComponent(GameObject *go) :
            Component(go)
        {}
    };
}
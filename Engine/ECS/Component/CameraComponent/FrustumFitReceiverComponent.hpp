#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class FrustumFitReceiverComponent : public Component {
    public:
        FrustumFitReceiverComponent(GameObject *go) :
            Component(go)
        {}
    };
}
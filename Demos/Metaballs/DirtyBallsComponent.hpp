#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace Metaballs {
    class DirtyBallsComponent : public Component {

    public:
        DirtyBallsComponent(GameObject* go) : Component(go) {}
    };

}

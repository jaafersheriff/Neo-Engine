#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct SelectedComponent : public Component {
        SelectedComponent(GameObject* go)
            : Component(go)
        {}
    };
}

#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace Metaballs {
    class MetaballComponent : public Component {

    public:
        MetaballComponent(GameObject* go);
    };
}

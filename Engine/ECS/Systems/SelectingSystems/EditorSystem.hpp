#pragma once

#include "ECS/Systems/System.hpp"
#include "SelectingSystem.hpp"

namespace neo {

    class EditorSystem : public SelectingSystem {

    public:
        EditorSystem();
    };

}

#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

class DirtyBallsComponent : public Component {

public:
    DirtyBallsComponent(GameObject* go) : Component(go) {}
};


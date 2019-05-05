#pragma once

#include "Component/Component.hpp"

using namespace neo;

class LookAtCameraReceiver : public Component {
public:
     LookAtCameraReceiver(GameObject *go) :
        Component(go)
    {}

};
#pragma once

#include "Component/Component.hpp"

class LookAtCameraReceiver : public neo::Component {
public:
     LookAtCameraReceiver(neo::GameObject *go) :
        neo::Component(go)
    {}

};
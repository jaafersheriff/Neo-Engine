#pragma once

#include "Component/Component.hpp"

class DecalRenderable: public neo::Component {
public:
     DecalRenderable(neo::GameObject *go) :
        neo::Component(go)
    {}

};
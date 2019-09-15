#pragma once

using namespace neo;

class SelectableComponent : public Component {
    public:
        SelectableComponent(GameObject *go) :
            Component(go)
        {}
};
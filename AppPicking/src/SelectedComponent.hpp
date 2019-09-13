#pragma once

using namespace neo;

class SelectedComponent : public Component {
    public:
        SelectedComponent(GameObject *go) :
            Component(go)
        {}
};
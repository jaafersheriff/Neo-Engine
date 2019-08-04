#pragma once

using namespace neo;

class MockOrthoComponent : public Component {
    public:
        float distance = 10.f;
        float range = 256.f;
        MockOrthoComponent(GameObject *go) :
            Component(go)
        {}
};
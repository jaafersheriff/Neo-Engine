#pragma once

using namespace neo;

class MockOrthoComponent : public Component {
    public:
        float distance;
        MockOrthoComponent(GameObject *go, float dist = 10.f) :
            Component(go), 
            distance(dist)
        {}
};
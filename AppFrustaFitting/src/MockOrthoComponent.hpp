#pragma once

using namespace neo;

class MockOrthoComponent : public Component {
    public:
        MockOrthoComponent(GameObject *go) :
            Component(go) 
        {}
};
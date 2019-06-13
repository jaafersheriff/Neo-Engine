#pragma once

using namespace neo;

class MockPerspectiveComponent : public Component {
    public:
        MockPerspectiveComponent(GameObject *go) :
            Component(go) 
        {}
};
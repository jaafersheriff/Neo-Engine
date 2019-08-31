#pragma once

#include "Systems/System.hpp"

namespace neo {

    class CameraControllerSystem : public System {

    public:
        CameraControllerSystem() :
            System("CameraController System")
        {}

        virtual void update(const float dt) override;
        virtual void imguiEditor() override;

    private:

        float mSuperSpeed = 2.5f;

    };
}
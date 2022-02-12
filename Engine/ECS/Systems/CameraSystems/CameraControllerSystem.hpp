#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

    class CameraControllerSystem : public System {

    public:
        CameraControllerSystem() :
            System("CameraController System")
        {}

        virtual void update(ECS& ecs) override;
        virtual void imguiEditor(ECS& ecs) override;

    protected:
        void _updateLook(const float dt, ECS&, CameraControllerComponent&);
        void _updatePosition(const float dt, ECS&, CameraControllerComponent&);

    private:

        float mSuperSpeed = 2.5f;

    };
}
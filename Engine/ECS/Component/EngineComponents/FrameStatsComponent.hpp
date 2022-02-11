#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

namespace neo {

    class FrameStatsComponent : public Component {

    public:
        FrameStatsComponent(GameObject *go, float runTime, float timeStep)
            : Component(go)
            , mRunTime(runTime)
            , mDT(timeStep)
        {}

        const float mRunTime;
        const float mDT;
    };
}
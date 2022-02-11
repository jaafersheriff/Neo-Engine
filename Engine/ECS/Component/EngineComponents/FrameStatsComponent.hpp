#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

namespace neo {

    class FrameStatsComponent : public Component {

    public:
        FrameStatsComponent(GameObject *go, float runTime)
            : Component(go)
            , mRunTime(runTime)
        {}

        const float mRunTime;
    };
}
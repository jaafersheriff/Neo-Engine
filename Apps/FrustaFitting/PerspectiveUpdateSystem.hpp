#pragma once

#include "ECS/Systems/System.hpp"
#include "Engine/Engine.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include <algorithm>
#include <limits>

using namespace neo;

class PerspectiveUpdateSystem : public System {

public:
    bool mUpdatePerspective = true;

    PerspectiveUpdateSystem() :
        System("PerspectiveUpdate System") {
    }

    virtual void update(const float dt) override {
        NEO_UNUSED(dt);
        auto sourceCamera = Engine::getComponentTuple<FrustumFitSourceComponent, SpatialComponent>();
        if (!sourceCamera) {
            return;
        }

        if (mUpdatePerspective) {
            if (auto frameStats = Engine::getSingleComponent<FrameStatsComponent>()) {
                float f = static_cast<float>(glm::sin(frameStats->mRunTime));
                float g = static_cast<float>(glm::cos(frameStats->mRunTime));
                sourceCamera->get<SpatialComponent>()->setLookDir(glm::vec3(f, f / 2, g));
            }
        }
    }
};

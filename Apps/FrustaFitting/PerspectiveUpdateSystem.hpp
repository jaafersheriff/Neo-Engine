#pragma once

#include "ECS/Systems/System.hpp"
#include "Engine.hpp"

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
        auto sourceCamera = Engine::getComponentTuple<FrustumFitSourceComponent, SpatialComponent>();
        if (!sourceCamera) {
            return;
        }

        if (mUpdatePerspective) {
            float f = static_cast<float>(glm::sin(Util::getRunTime()));
            float g = static_cast<float>(glm::cos(Util::getRunTime()));
            sourceCamera->get<SpatialComponent>()->setLookDir(glm::vec3(f, f / 2, g));
        }
    }
};

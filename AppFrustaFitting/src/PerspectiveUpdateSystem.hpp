#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

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
        const auto mainCamera = Engine::getSingleComponent<MainCameraComponent>();
        if (!mainCamera) {
            return;
        }
        auto perspectiveSpat = mainCamera->getGameObject().getComponentByType<SpatialComponent>();
        if (!perspectiveSpat) {
            return;
        }

        if (mUpdatePerspective) {
            float f = glm::sin(Util::getRunTime());
            float g = glm::cos(Util::getRunTime());
            perspectiveSpat->setLookDir(glm::vec3(f, f / 2, g));
        }
    }
};

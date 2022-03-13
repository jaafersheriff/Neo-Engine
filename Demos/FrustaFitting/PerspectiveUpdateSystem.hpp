#pragma once

#include "ECS/Systems/System.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include <algorithm>
#include <limits>

using namespace neo;

namespace FrustaFitting {
    class PerspectiveUpdateSystem : public System {

    public:
        bool mUpdatePerspective = true;

        PerspectiveUpdateSystem() :
            System("PerspectiveUpdate System") {
        }

        virtual void update(ECS& ecs) override {
            if (auto sourceCamera = ecs.getSingleView<FrustumFitSourceComponent, SpatialComponent>()) {
                if (mUpdatePerspective) {
                    if (auto frameStats = ecs.getComponent<FrameStatsComponent>()) {
                        auto&& [_, __, sourceSpatial] = *sourceCamera;
                        float f = static_cast<float>(glm::sin(std::get<1>(*frameStats).mRunTime));
                        float g = static_cast<float>(glm::cos(std::get<1>(*frameStats).mRunTime));
                        sourceSpatial.setLookDir(glm::vec3(f, f / 2, g));
                    }
                }
            }
        }
    };
}

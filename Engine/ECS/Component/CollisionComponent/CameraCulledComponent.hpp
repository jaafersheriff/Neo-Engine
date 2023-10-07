#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/Component.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"

namespace neo {

    struct CameraCulledComponent : public Component {
        virtual std::string getName() const override { return "ObjectInMainViewComponent"; } 

        bool isInView(const ECS& ecs, ECS::Entity thisID, ECS::Entity cameraID) const {
            if (ecs.isSystemEnabled<FrustumSystem>() && ecs.isSystemEnabled<FrustumCullingSystem>() && ecs.has<BoundingBoxComponent>(thisID)) {
                return mCameraViews.find(cameraID) != mCameraViews.end();
            }

            return true;
        }

        std::set<ECS::Entity> mCameraViews;
    };
}
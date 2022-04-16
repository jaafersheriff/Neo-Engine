#pragma once

#include "ECS/Component/Component.hpp"

#include "PerspectiveCameraComponent.hpp"
#include "OrthoCameraComponent.hpp"

namespace neo {

    struct MainCameraComponent : public Component {
        virtual std::string getName() const override { return "MainCameraComponent"; }
        // MainCameraComponent(GameObject* go)
        //     : Component(go)
        // {
        //     Messenger::addReceiver<FrameSizeMessage>(nullptr, [](const Message& m, ECS& ecs) {
        //         const FrameSizeMessage& msg = static_cast<const FrameSizeMessage&>(m);
        //         auto comp = ecs.getSingleComponent<MainCameraComponent>();
        //         if (auto camera = dynamic_cast<PerspectiveCameraComponent*>(comp->getGameObject().getComponentByType<CameraComponent>())) {
        //             NEO_LOG("Updating main camera aspect ratio");
        //             camera->setAspectRatio(msg.mSize.x / static_cast<float>(msg.mSize.y));
        //         }
        //     });
        // }
    };
}
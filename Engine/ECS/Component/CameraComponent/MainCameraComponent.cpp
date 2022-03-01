#include "MainCameraComponent.hpp"
#include "Engine/Engine.hpp"

#include "PerspectiveCameraComponent.hpp"
#include "ECS/GameObject.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    MainCameraComponent::MainCameraComponent(GameObject *gameObject) :
        Component(gameObject) {
            Messenger::addReceiver<FrameSizeMessage>(nullptr, [](const Message& m, ECS& ecs) {
                const FrameSizeMessage& msg = static_cast<const FrameSizeMessage&>(m);
                auto comp = ecs.getSingleComponent<MainCameraComponent>();
                if (auto camera = dynamic_cast<PerspectiveCameraComponent*>(comp->getGameObject().getComponentByType<CameraComponent>())) {
                    // camera->setAspectRatio(msg.mFrameSize.x / static_cast<float>(msg.mFrameSize.y));
                    camera->setAspectRatio(1.17f); NEO_UNUSED(msg);
                }
            });
    }
}

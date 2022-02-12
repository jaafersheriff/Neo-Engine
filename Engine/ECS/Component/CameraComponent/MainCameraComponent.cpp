#include "MainCameraComponent.hpp"
#include "Engine/Engine.hpp"

#include "PerspectiveCameraComponent.hpp"
#include "ECS/GameObject.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    MainCameraComponent::MainCameraComponent(GameObject *gameObject) :
        Component(gameObject) {
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [](const Message& m, ECS& ecs) {
                const WindowFrameSizeMessage& msg = static_cast<const WindowFrameSizeMessage&>(m);
                auto comp = ecs.getSingleComponent<MainCameraComponent>();
                if (auto camera = dynamic_cast<PerspectiveCameraComponent*>(comp->getGameObject().getComponentByType<CameraComponent>())) {
                    camera->setAspectRatio(msg.mFrameSize.x / static_cast<float>(msg.mFrameSize.y));
                }
            });
    }
}

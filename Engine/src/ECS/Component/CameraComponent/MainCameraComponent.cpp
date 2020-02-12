#include "MainCameraComponent.hpp"
#include <Engine.hpp>

#include "PerspectiveCameraComponent.hpp"
#include "ECS/GameObject.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    MainCameraComponent::MainCameraComponent(GameObject *gameObject) :
        Component(gameObject) {
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [](const Message& _msg) {
                const WindowFrameSizeMessage& msg = static_cast<const WindowFrameSizeMessage &>(_msg);
                auto comp = Engine::getSingleComponent<MainCameraComponent>();
                if (auto camera = dynamic_cast<PerspectiveCameraComponent*>(comp->getGameObject().getComponentByType<CameraComponent>())) {
                    camera->setAspectRatio(Window::getAspectRatio());
                }
            });
    }
}

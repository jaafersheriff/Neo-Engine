#include "MetaballComponent.hpp"
#include "MetaballsSystem.hpp"

#include "ECS/Component/EngineComponents/SingleFrameComponent.hpp"

using namespace neo;

MetaballComponent::MetaballComponent(GameObject* go) :
    Component(go) {
        Messenger::addReceiver<SpatialChangeMessage>(go, [](const Message& _msg, ECS& ecs) {
            NEO_UNUSED(_msg);
            GameObject* gameObject = &ecs.createGameObject();
            ecs.addComponent<DirtyBallsComponent>(gameObject);
            ecs.addComponent<SingleFrameComponent>(gameObject);
        });
}
#include "MetaballComponent.hpp"
#include "MetaballsSystem.hpp"
#include "Engine/Engine.hpp"

using namespace neo;

MetaballComponent::MetaballComponent(GameObject* go) :
    Component(go) {
        Messenger::addReceiver<SpatialChangeMessage>(go, [](const Message& _msg, ECS& ecs) {
            const SpatialChangeMessage& msg = static_cast<const SpatialChangeMessage &>(_msg);
            NEO_UNUSED(msg);
            GameObject* gameObject = &ecs.createGameObject();
            ecs.addComponent<DirtyBallsComponent>(gameObject);
        });
}
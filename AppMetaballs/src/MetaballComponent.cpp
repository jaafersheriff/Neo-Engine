#include "MetaballComponent.hpp"
#include "MetaballsSystem.hpp"
#include "Engine.hpp"

using namespace neo;

MetaballComponent::MetaballComponent(GameObject* go) :
    Component(go) {
        Messenger::addReceiver<SpatialChangeMessage>(nullptr, [](const Message& _msg) {
            const SpatialChangeMessage& msg = static_cast<const SpatialChangeMessage &>(_msg);
            Engine::getSystem<MetaballsSystem>().mDirtyBalls = true;
        });
}
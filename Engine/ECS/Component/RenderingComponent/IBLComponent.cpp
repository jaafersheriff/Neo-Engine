#include "ECS/pch.hpp"
#include "ECS/Component/RenderingComponent/IBLComponent.hpp"

#include "ECS/ECS.hpp"

#include "Messaging/Message.hpp"
#include "Messaging/Messenger.hpp"

namespace neo {

	void IBLComponent::registerMessageHandlers(ECS& ecs) {
		// The ECS goes in as EnTT's delegate payload, which is what lets these stay plain statics on
		// the component instead of members of something that had to be handed an ECS.
		Messenger::removeReceiver<IBLConvolvedMessage, &IBLComponent::onConvolved>(ecs);
		Messenger::addReceiver<IBLConvolvedMessage, &IBLComponent::onConvolved>(ecs);
		Messenger::removeReceiver<IBLDFGLutGeneratedMessage, &IBLComponent::onDFGLutGenerated>(ecs);
		Messenger::addReceiver<IBLDFGLutGeneratedMessage, &IBLComponent::onDFGLutGenerated>(ecs);
	}

	void IBLComponent::onConvolved(ECS& ecs, const IBLConvolvedMessage& message) {
		if (auto* ibl = ecs.getComponent<IBLComponent>(message.mEntity)) {
			ibl->mConvolvedSkybox = message.mConvolvedSkybox;
			ibl->mConvolved = true;
		}
	}

	void IBLComponent::onDFGLutGenerated(ECS& ecs, const IBLDFGLutGeneratedMessage& message) {
		if (auto* ibl = ecs.getComponent<IBLComponent>(message.mEntity)) {
			ibl->mDFGLut = message.mDFGLut;
			ibl->mDFGGenerated = true;
		}
	}
}

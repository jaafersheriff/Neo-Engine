#include "ECS/pch.hpp"
#include "ECS/Component/RenderingComponent/IBLComponent.hpp"

#include "ECS/ECS.hpp"

#include "Messaging/Message.hpp"
#include "Messaging/Messenger.hpp"

namespace neo {

	void IBLComponent::registerMessageHandlers(ECS& ecs) {
		// Method for render thread to communicate certain jobs were completed..
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

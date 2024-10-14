#include "ECS/pch.hpp"
#include "FrustumToLineSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

namespace neo {

	void FrustumToLineSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		TRACY_ZONEN("FrustumToLineSystem");
		for (auto&& [entity, line, frustum] : ecs.getView<LineMeshComponent, FrustumComponent>().each()) {

			line.clearNodes();

			// We have to do this ridiculousness because line strip (:
			line.addNode(frustum.mNearLeftBottom);
			line.addNode(frustum.mNearLeftTop);
			line.addNode(frustum.mNearRightTop);
			line.addNode(frustum.mNearRightBottom);
			line.addNode(frustum.mNearLeftBottom);
			line.addNode(frustum.mFarLeftBottom);
			line.addNode(frustum.mFarLeftTop);
			line.addNode(frustum.mNearLeftTop);
			line.addNode(frustum.mFarLeftTop);
			line.addNode(frustum.mFarRightTop);
			line.addNode(frustum.mNearRightTop);
			line.addNode(frustum.mFarRightTop);
			line.addNode(frustum.mFarRightBottom);
			line.addNode(frustum.mNearRightBottom);
			line.addNode(frustum.mFarRightBottom);
			line.addNode(frustum.mFarLeftBottom);
		}
	}
}

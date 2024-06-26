#include "ECS/pch.hpp"
#include "FrustumToLineSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

namespace neo {

	void FrustumToLineSystem::update(ECS& ecs) {
		TRACY_ZONEN("FrustumToLineSystem");
		for (auto&& [entity, line, frustum] : ecs.getView<LineMeshComponent, FrustumComponent>().each()) {

			line.clearNodes();

			// We have to do this ridiculousness because line strip (:
			line.addNode(frustum.NearLeftBottom);
			line.addNode(frustum.NearLeftTop);
			line.addNode(frustum.NearRightTop);
			line.addNode(frustum.NearRightBottom);
			line.addNode(frustum.NearLeftBottom);
			line.addNode(frustum.FarLeftBottom);
			line.addNode(frustum.FarLeftTop);
			line.addNode(frustum.NearLeftTop);
			line.addNode(frustum.FarLeftTop);
			line.addNode(frustum.FarRightTop);
			line.addNode(frustum.NearRightTop);
			line.addNode(frustum.FarRightTop);
			line.addNode(frustum.FarRightBottom);
			line.addNode(frustum.NearRightBottom);
			line.addNode(frustum.FarRightBottom);
			line.addNode(frustum.FarLeftBottom);
		}
	}
}

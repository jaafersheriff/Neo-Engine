#pragma once

#include "ECS/Systems/System.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Util/Profiler.hpp"

#include <algorithm>
#include <limits>

using namespace neo;

namespace CSM {
	class PerspectiveUpdateSystem : public System {

	public:
		PerspectiveUpdateSystem() :
			System("PerspectiveUpdate System") {
		}

		virtual void update(ECS& ecs, const ResourceManagers& resourceManagers) override {
			NEO_UNUSED(resourceManagers);
			TRACY_ZONEN("PerspectiveUpdateSystem");
			if (auto sourceCamera = ecs.getSingleView<FrustumFitSourceComponent, SpatialComponent>()) {
				if (auto frameStats = ecs.getComponent<FrameStatsComponent>()) {
					mOffset += std::get<1>(*frameStats).mDT * mSpeed;
					auto&& [_, __, sourceSpatial] = *sourceCamera;
					float f = static_cast<float>(glm::sin(mOffset));
					float g = static_cast<float>(glm::cos(mOffset));
					sourceSpatial.setLookDir(glm::vec3(f, f / 2, g));
				}
			}
		}

		virtual void imguiEditor(ECS&) override {
			ImGui::SliderFloat("Speed", &mSpeed, 0.f, 3.f);
		};
	private: 
		float mOffset = 0.f;
		float mSpeed = 1.f;
	};
}

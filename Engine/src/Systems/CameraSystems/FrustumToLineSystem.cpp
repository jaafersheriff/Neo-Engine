#include <Engine.hpp>

#include "FrustumToLineSystem.hpp"

namespace neo {

    void FrustumToLineSystem::update(const float dt) {
        for (auto camera : Engine::getComponentTuples<CameraComponent, LineMeshComponent, FrustumComponent>()) {

            glm::vec3 color(1.f);
            if (auto material = camera.mGameObject.getComponentByType<MaterialComponent>()) {
                color = material->mDiffuse;
            }

            auto line = camera.get<LineMeshComponent>();
            auto bounds = camera.get<FrustumComponent>();

            line->clearNodes();

            // We have to do this ridiculousness because line strip (:
            line->addNode(bounds->NearLeftBottom, color);
            line->addNode(bounds->NearLeftTop, color);
            line->addNode(bounds->NearRightTop, color);
            line->addNode(bounds->NearRightBottom, color);
            line->addNode(bounds->NearLeftBottom, color);
            line->addNode(bounds->FarLeftBottom, color);
            line->addNode(bounds->FarLeftTop, color);
            line->addNode(bounds->NearLeftTop, color);
            line->addNode(bounds->FarLeftTop, color);
            line->addNode(bounds->FarRightTop, color);
            line->addNode(bounds->NearRightTop, color);
            line->addNode(bounds->FarRightTop, color);
            line->addNode(bounds->FarRightBottom, color);
            line->addNode(bounds->NearRightBottom, color);
            line->addNode(bounds->FarRightBottom, color);
            line->addNode(bounds->FarLeftBottom, color);
        }
    }
}

#include <Engine.hpp>
#include "FrustumPlanesSystem.hpp"

namespace neo {
    void FrustumPlanesSystem::update(const float dt) {
        for (auto camera : Engine::getComponents<CameraComponent>()) {
            if (auto planesComp = camera->getGameObject().getComponentByType<FrustumPlanesComponent>()) {
                glm::mat4 PV = camera->getProj() * camera->getView();

                planesComp->mLeft.x = PV[0][3] + PV[0][0];
                planesComp->mLeft.y = PV[1][3] + PV[1][0];
                planesComp->mLeft.z = PV[2][3] + PV[2][0];
                planesComp->mLeft.w = PV[3][3] + PV[3][0];
                planesComp->mLeft /= glm::length(glm::vec3(planesComp->mLeft));

                planesComp->mRight.x = PV[0][3] - PV[0][0];
                planesComp->mRight.y = PV[1][3] - PV[1][0];
                planesComp->mRight.z = PV[2][3] - PV[2][0];
                planesComp->mRight.w = PV[3][3] - PV[3][0];
                planesComp->mRight /= glm::length(glm::vec3(planesComp->mRight));
                
                planesComp->mBottom.x = PV[0][3] + PV[0][1];
                planesComp->mBottom.y = PV[1][3] + PV[1][1];
                planesComp->mBottom.z = PV[2][3] + PV[2][1];
                planesComp->mBottom.w = PV[3][3] + PV[3][1];
                planesComp->mBottom /= glm::length(glm::vec3(planesComp->mBottom));
                
                planesComp->mTop.x = PV[0][3] - PV[0][1];
                planesComp->mTop.y = PV[1][3] - PV[1][1];
                planesComp->mTop.z = PV[2][3] - PV[2][1];
                planesComp->mTop.w = PV[3][3] - PV[3][1];
                planesComp->mTop /= glm::length(glm::vec3(planesComp->mTop));
                
                planesComp->mFar.x = PV[0][3] - PV[0][2];
                planesComp->mFar.y = PV[1][3] - PV[1][2];
                planesComp->mFar.z = PV[2][3] - PV[2][2];
                planesComp->mFar.w = PV[3][3] - PV[3][2];
                planesComp->mFar /= glm::length(glm::vec3(planesComp->mFar));
                
                planesComp->mNear.x = PV[0][2];
                planesComp->mNear.y = PV[1][2];
                planesComp->mNear.z = PV[2][2];
                planesComp->mNear.w = PV[3][2];
                planesComp->mNear /= glm::length(glm::vec3(planesComp->mNear));
            }
        }
    }
}

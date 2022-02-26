#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Mesh.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/WireframeRenderable.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Util/Util.hpp"


using namespace neo;

namespace FrustaFitting {
    class WireShader : public Shader {

    public:
        WireShader() :
            Shader("Wire Shader",
                R"(
                        layout (location = 0) in vec3 vertPos;
                        uniform mat4 P, V, M;
                        void main() {
                            gl_Position = P * V * M * vec4(vertPos, 1);
                        })",
                R"(
                        uniform vec3 wireColor;
                        out vec4 color;
                        void main() {
                            color = vec4(wireColor, 1.0);
                        })"
            )
        {}

        virtual void render(const ECS& ecs) override {
            bind();
            CHECK_GL(glDisable(GL_CULL_FACE));
            CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

            /* Load PV */
            auto mainCamera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(mainCamera, "No main camera exists");
            loadUniform("P", mainCamera->get<CameraComponent>()->getProj());
            loadUniform("V", mainCamera->get<CameraComponent>()->getView());

            auto mockCamera = ecs.getComponentTuple<FrustumComponent, FrustumFitSourceComponent, CameraComponent>();
            const auto& cameraFrustum = mockCamera->mGameObject.getComponentByType<FrustumComponent>();

            for (auto& renderableIt : ecs.getComponentTuples<MeshComponent, SpatialComponent>()) {
                const auto spatialComponent = renderableIt->get<SpatialComponent>();
                glm::vec3 color(1.f);

                glm::vec3 position = spatialComponent->getPosition();
                glm::vec3 _scale = spatialComponent->getOrientation() * spatialComponent->getScale();
                float scale = std::max(_scale.x, std::max(_scale.y, _scale.z));
				// VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("WireShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderableIt->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        position += boundingBox->getCenter();
                        scale *= boundingBox->getRadius();
                        if (!cameraFrustum->isInFrustum(position, scale)) {
                            continue;
                        }
                    }
                }


                glm::mat4 M = glm::scale(glm::translate(glm::mat4(1.f), position) * glm::mat4(spatialComponent->getOrientation()), glm::vec3(scale));
                loadUniform("M", M);
                loadUniform("wireColor", color);

                /* Draw outline */
                Library::getMesh("sphere").mMesh->draw();
            }

            unbind();
        }
    };
}

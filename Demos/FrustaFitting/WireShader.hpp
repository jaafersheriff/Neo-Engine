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
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            /* Load PV */
            auto cameraView = ecs.getSingleView<MainCameraComponent, SpatialComponent>();
            if (cameraView) {
                auto&& [cameraEntity, _, spatial] = *cameraView;
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                loadUniform("V", spatial.getView());
            }

            auto mockCamera = ecs.getSingleView<FrustumComponent, FrustumFitSourceComponent>();

            ecs.getView<MeshComponent, SpatialComponent>().each([&](const ECS::Entity e, const MeshComponent& mesh, const SpatialComponent& spatial) {
                glm::vec3 color(1.f);

                glm::vec3 position = spatial.getPosition();
                glm::vec3 _scale = spatial.getOrientation() * spatial.getScale();
                float scale = std::max(_scale.x, std::max(_scale.y, _scale.z));
				// VFC
                if (mockCamera) {
                    auto&& [_, cameraFrustum, __] = *mockCamera;
                    MICROPROFILE_SCOPEI("WireShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = ecs.cGetComponent<BoundingBoxComponent>(e)) {
                        position += boundingBox->getCenter();
                        scale *= boundingBox->getRadius();
                        if (!cameraFrustum.isInFrustum(position, scale)) {
                            return;
                        }
                    }
                }


                glm::mat4 M = glm::scale(glm::translate(glm::mat4(1.f), position) * glm::mat4(spatial.getOrientation()), glm::vec3(scale));
                loadUniform("M", M);
                loadUniform("wireColor", color);

                /* Draw outline */
                mesh.mMesh->draw();
            });

            unbind();
        }
    };
}

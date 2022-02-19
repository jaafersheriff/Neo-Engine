#pragma once

#include "Engine/Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/RenderableComponent/OutlineRenderable.hpp"

namespace neo {

    class OutlineShader : public Shader {

    public:

        OutlineShader() :
            Shader("Outline Shader",
                R"(
                layout(location = 0) in vec3 vertPos;
                uniform mat4 P, V, M;
                out vec2 fragTex;
                void main() {
                    gl_Position = P * V * M * vec4(vertPos, 1.0);
                })",
                R"(
                uniform vec4 outlineColor;
                out vec4 color;
                void main() {
                    color = outlineColor;
                })")
        {}

        virtual void render(const ECS& ecs) override {
            bind();

            CHECK_GL(glCullFace(GL_FRONT));

            /* Load PV */
            auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            const auto cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();

            for (auto& renderable : ecs.getComponentTuples<renderable::OutlineRenderable, MeshComponent, SpatialComponent>()) {
                auto renderableOutline = renderable->get<renderable::OutlineRenderable>();
                auto renderableSpatial = renderable->get<SpatialComponent>();

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("OutlineShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        if (!cameraFrustum->isInFrustum(renderableSpatial->getModelMatrix() * glm::vec4(boundingBox->getCenter(), 1.f), boundingBox->getRadius())) {
                            continue;
                        }
                    }
                }

                // Scale from local origin
                glm::vec3 scaleFactor = renderableSpatial->getScale() * (1.f + renderableOutline->mScale);
                glm::mat4 S;
                if (auto bb = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                    glm::vec3 worldSpaceCenter = renderableSpatial->getModelMatrix() * glm::vec4(bb->getCenter(), 1.f);
                    glm::vec3 offsetTranslation = renderableSpatial->getPosition() - worldSpaceCenter;
                    // Set model's true center as the origin
                    S = glm::translate(glm::mat4(1.f), -offsetTranslation);
                    // Scale
                    S *= glm::scale(glm::mat4(1.f), glm::vec3(renderableSpatial->getScale() * (renderableOutline->mScale + 1.f)));
                    // Translate back
                    S *= glm::translate(glm::mat4(1.f), offsetTranslation);
                }
                else {
                    S = glm::scale(glm::mat4(1.f), scaleFactor);
                }

                glm::mat4 T = glm::translate(glm::mat4(1.f), renderableSpatial->getPosition());
                glm::mat4 R = glm::mat4(renderableSpatial->getOrientation());
                glm::mat4 M = T * S * R;
                loadUniform("M", M);

                loadUniform("outlineColor", renderableOutline->mColor);

                /* DRAW */
                renderable->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
        }
    };
}

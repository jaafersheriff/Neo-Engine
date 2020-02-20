#pragma once

#include "Engine.hpp"

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GlHelper.hpp"

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

        virtual void render() override {
            bind();

            CHECK_GL(glCullFace(GL_FRONT));

            /* Load PV */
            auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            const auto cameraFrustum = camera->mGameObject.getComponentByType<FrustumComponent>();

            for (auto& renderable : Engine::getComponentTuples<renderable::OutlineRenderable, MeshComponent, SpatialComponent>()) {
                auto renderableOutline = renderable->get<renderable::OutlineRenderable>();
                auto renderableSpatial = renderable->get<SpatialComponent>();

                // VFC
                if (cameraFrustum) {
                    MICROPROFILE_SCOPEI("OutlineShader", "VFC", MP_AUTO);
                    if (const auto& boundingBox = renderable->mGameObject.getComponentByType<BoundingBoxComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!cameraFrustum->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }

                glm::mat4 M = renderableSpatial->getModelMatrix() * glm::scale(glm::mat4(1.f), glm::vec3(1.f + renderableOutline->mScale));
                loadUniform("M", M);

                loadUniform("outlineColor", renderableOutline->mColor);

                /* DRAW */
                renderable->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
        }
    };
}

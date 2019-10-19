#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GlHelper.hpp"

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

        virtual void render(const CameraComponent &camera) override {

            bind();

            CHECK_GL(glCullFace(GL_FRONT));

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            for (const auto& renderable : Engine::getComponents<renderable::OutlineRenderable>()) {
                const auto& renderableMesh = renderable->getGameObject().getComponentByType<MeshComponent>();
                const auto& renderableSpatial = renderable->getGameObject().getComponentByType<SpatialComponent>();
                if (!renderableMesh || !renderableSpatial) {
                    continue;
                }

                // VFC
                if (const auto& boundingBox = renderable->getGameObject().getComponentByType<BoundingBoxComponent>()) {
                    if (const auto& frustumPlanes = camera.getGameObject().getComponentByType<FrustumComponent>()) {
                        float radius = glm::max(glm::max(renderableSpatial->getScale().x, renderableSpatial->getScale().y), renderableSpatial->getScale().z) * boundingBox->getRadius();
                        if (!frustumPlanes->isInFrustum(renderableSpatial->getPosition(), radius)) {
                            continue;
                        }
                    }
                }

                /* Bind mesh */
                const Mesh & mesh(renderableMesh->getMesh());
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));

                glm::mat4 M = renderableSpatial->getModelMatrix() * glm::scale(glm::mat4(1.f), glm::vec3(1.f + renderable->mScale));

                loadUniform("M", M);
                loadUniform("outlineColor", renderable->mColor);

                /* DRAW */
                mesh.draw();
            }

            unbind();
        }
    };
}

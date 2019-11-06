#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

namespace neo {

    class WireframeShader : public Shader {

        public:
            WireframeShader() :
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

            virtual void render(const CameraComponent &camera) {
                bind();
                CHECK_GL(glDisable(GL_CULL_FACE));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

                /* Load PV */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());

                for (auto& renderable : Engine::getComponentTuples<renderable::WireframeRenderable, MeshComponent, SpatialComponent>()) {
                    const auto meshComponent = renderable->get<MeshComponent>();
                    const auto spatialComponent = renderable->get<SpatialComponent>();

                    /* Bind mesh */
                    const Mesh & mesh(meshComponent->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.mVAOID));

                    loadUniform("M", spatialComponent->getModelMatrix());

                    glm::vec3 color(1.f);
                    if (const auto material = renderable->mGameObject.getComponentByType<MaterialComponent>()) {
                        color = material->mDiffuse;
                    }
                    loadUniform("wireColor", color);

                    /* Draw outline */
                    mesh.draw();
                }

                unbind();
            }
        };

}
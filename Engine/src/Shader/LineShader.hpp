#pragma once

#include "Engine.hpp"

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Component/ModelComponent/LineMeshComponent.hpp"

namespace neo {

    class LineShader : public Shader {

        public:
            LineShader() :
                Shader("Line Shader",
                        R"(
                        layout (location = 0) in vec3 vertPos;
                        uniform mat4 P, V, M;
                        void main() {
                            gl_Position = P * V * M * vec4(vertPos, 1);
                        })",
                        R"(
                        uniform vec3 lineColor;
                        out vec4 color;
                        void main() {
                            color = vec4(lineColor, 1.0);
                        })"
                )
            {}

            virtual void render(const CameraComponent &camera) override {
                bind();

                /* Load PV */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());
                loadUniform("M", camera.getView());

                for (auto& renderable : Engine::getComponents<renderable::LineMeshComponent>()) {
                    /* Bind mesh */
                    const Mesh & mesh(renderable->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.mVAOID));
                    CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.mVertexBufferID));

                    auto spatial = renderable->getGameObject().getSpatial();
                    loadUniform("M", spatial ? spatial->getModelMatrix() : glm::mat4(1.f));

                    loadUniform("lineColor", renderable->mLine->mColor);

                    mesh.draw(renderable->mLine->getNodes().size());
                }


                CHECK_GL(glBindVertexArray(0));	
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                unbind();
            }
        };

}
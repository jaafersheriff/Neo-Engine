#pragma once

#include "NeoEngine.hpp"

#include "Shader/Shader.hpp"
#include "MasterRenderer/MasterRenderer.hpp"
#include "GLHelper/GLHelper.hpp"

#include "Component/AnimationComponent/LineRenderable.hpp"

namespace neo {

    class LineShader : public Shader {

        public:
            LineShader() :
                Shader("Line Shader",
                        "\
                        #version 330 core\n\
                        layout (location = 0) in vec3 vertPos;\
                        uniform mat4 P, V, M;\
                        void main() {\
                            gl_Position = P * V * M * vec4(vertPos, 1);\
                        }",
                        "\
                        #version 330 core\n\
                        uniform vec3 lineColor;\
                        out vec4 color;\
                        void main() {\
                            color = vec4(lineColor, 1.0);\
                        }"
                )
            {}

            virtual void render(const CameraComponent &camera) override {
                bind();

                /* Load PV */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());

                for (auto & lineR : MasterRenderer::getRenderables<LineShader, LineRenderable>()) {
                    /* Bind mesh */
                    const Mesh & mesh(lineR->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.vaoId));
                    CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, mesh.vertBufId));

                    auto spatial = lineR->getGameObject().getSpatial();
                    loadUniform("M", spatial ? spatial->getModelMatrix() : glm::mat4(1.f));
                    loadUniform("lineColor", lineR->line->lineColor);

                    mesh.draw(lineR->line->getNodes().size());
                }

                CHECK_GL(glBindVertexArray(0));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                unbind();
            }
        };

}
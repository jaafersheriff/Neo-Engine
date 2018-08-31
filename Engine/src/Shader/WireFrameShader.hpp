#pragma once

#include "Shader/Shader.hpp"
#include "MasterRenderer/MasterRenderer.hpp"
#include "GLHelper/GLHelper.hpp"

namespace neo {

    class WireframeShader : public Shader {

        public:
            WireframeShader() :
                Shader("Wire Shader",
                        "\
                        #version 330 core\n\
                        layout (location = 0) in vec3 vertPos;\
                        uniform mat4 P, V, M;\
                        void main() {\
                            gl_Position = P * V * M * vec4(vertPos, 1);\
                        }",
                        "\
                        #version 330 core\n\
                        out vec4 color;\
                        void main() {\
                            color = vec4(1.0);\
                        }"
                )
            {}

            virtual void render(const CameraComponent &camera) {
                bind();

                /* Load PV */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());

                for (auto r : MasterRenderer::getRenderables<WireframeShader, RenderableComponent>()) {
                    /* Bind mesh */
                    const Mesh & mesh(r->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.vaoId));
                    CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                    loadUniform("M", r->getGameObject().getSpatial()->getModelMatrix());

                    /* Draw outline */
                    CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                    mesh.draw();
                    CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
                }

                CHECK_GL(glBindVertexArray(0));
                CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
                unbind();
            }
        };

}
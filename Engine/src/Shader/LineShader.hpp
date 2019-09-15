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
                    layout (location = 1) in vec3 vertColor;
                    uniform mat4 P, V, M;
                    out vec3 vCol;
                    void main() {
                        gl_Position = P * V * vec4(vertPos, 1);
                        vCol = vertColor;
                    })",
                    R"(
                    in vec3 vCol;
                    out vec4 color;
                    void main() {
                        color = vec4(vCol, 1.0);
                    })"
                )
            {}

            virtual void render(const CameraComponent &camera) override {
                bind();

                /* Load PV */
                loadUniform("P", camera.getProj());
                loadUniform("V", camera.getView());

                for (auto& line : Engine::getComponents<LineMeshComponent>()) {
                    /* Bind mesh */
                    const Mesh & mesh(line->getMesh());
                    CHECK_GL(glBindVertexArray(mesh.mVAOID));

                    mesh.draw(line->getNodes().size());
                }

                CHECK_GL(glBindVertexArray(0));	
                unbind();
            }
        };

}
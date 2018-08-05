#pragma once

#include "Shader/Shader.hpp"
#include "Util/GLHelper.hpp"

using namespace neo;

class WireShader : public Shader {

private:
    public:
        WireShader(std::string res) :
            Shader("Wire Shader", 
                _strdup("\
                    #version 330 core\n\
                    layout (location = 0) in vec3 vertPos;\
                    layout (location = 1) in vec3 vertNor;\
                    layout (location = 2) in vec2 vertTex;\
                    uniform mat4 P;\
                    uniform mat4 V;\
                    uniform mat4 M;\
                    void main() {\
                        vec4 worldPos = M * vec4(vertPos, 1.0);\
                        gl_Position = P * V * worldPos;\
                    }"), 
                _strdup("\
                    #version 330 core\n\
                    out vec4 color;\
                    void main() {\
                        color = vec4(1.0);\
                    }")
            )
        {}
        
        virtual void render(float dt, const RenderSystem &renderSystem) override {
            bind();

            /* Load PV */
            const std::vector<CameraComponent *> cameras = NeoEngine::getComponents<CameraComponent>();
            if (cameras.size()) {
                loadMatrix(getUniform("P"), cameras.at(0)->getProj());
                loadMatrix(getUniform("V"), cameras.at(0)->getView());
            }

            for (auto r : renderSystem.getRenderables<WireShader, RenderableComponent>()) {
                /* Bind mesh */
                const Mesh & mesh(*r->getMesh());
                CHECK_GL(glBindVertexArray(mesh.vaoId));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.eleBufId));

                loadMatrix(getUniform("M"), r->getGameObject().getSpatial()->getModelMatrix());

                /* Draw outline */
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                CHECK_GL(glDrawElements(GL_TRIANGLES, (int)mesh.eleBufSize, GL_UNSIGNED_INT, nullptr));
                CHECK_GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            unbind();
        }
};
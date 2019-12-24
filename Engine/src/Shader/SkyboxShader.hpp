#pragma once

#include "Engine.hpp"
#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

namespace neo {
    class SkyboxShader : public Shader {

    public:

        SkyboxShader() :
            Shader("Skybox Shader",
                R"(
                layout (location = 0) in vec3 vertPos;
                uniform mat4 P;
                uniform mat4 V;
                out vec3 fragTex;
                void main() {
                    mat4 skyV = V;
                    skyV[3][0] = skyV[3][1] = skyV[3][2] = 0.0;
                    vec4 pos = P * skyV * vec4(vertPos, 1.0); 
                    gl_Position = pos.xyww;
                    fragTex = vertPos;
                }
                )", R"(
                in vec3 fragTex;
                uniform samplerCube cubeMap;
                out vec4 color;
                void main() {
                    color = texture(cubeMap, fragTex);
                }
                )")
        {}

        virtual void render(const CameraComponent &camera) override {
            auto skybox = Engine::getComponentTuple<renderable::SkyboxComponent, CubeMapComponent>();
            if (!skybox) {
                return;
            }

            CHECK_GL(glDisable(GL_CULL_FACE));
            CHECK_GL(glDepthFunc(GL_LEQUAL));
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            /* Bind texture */
            loadTexture("cubeMap", skybox->get<CubeMapComponent>()->mTexture);

            /* Draw */
            Library::getMesh("cube")->draw();

            unbind();
        }
    };
}
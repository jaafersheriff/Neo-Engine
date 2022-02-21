#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/RenderableComponent/SkyboxComponent.hpp"

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

        virtual void render(const ECS& ecs) override {
            auto skybox = ecs.getSingleComponent<renderable::SkyboxComponent>();
            if (!skybox) {
                return;
            }

            CHECK_GL(glDisable(GL_CULL_FACE));
            CHECK_GL(glDepthFunc(GL_LEQUAL));
            bind();

            /* Load PV */
            auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            /* Bind texture */
            loadTexture("cubeMap", skybox->mCubeMap);

            /* Draw */
            Library::getMesh("cube").mesh->draw();

            unbind();
        }
    };
}
#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Engine/Engine.hpp"

namespace neo {

    class SelectableShader : public Shader {

        public:

            SelectableShader() :
                Shader("Selectable",
                    R"(
                        layout (location = 0) in vec3 vertPos;
                        uniform mat4 P, V, M;
                        out vec2 fragTex;
                        void main() { gl_Position = P * V * M * vec4(vertPos, 1); })",
                    R"(
                        uniform int componentID;
                        out int color;
                        void main() {
                            color = componentID;
                        })"
                    ) {
                /* Init shadow map */
                Framebuffer *stencilBuffer = Library::createFBO("selectable");
                stencilBuffer->attachStencilTexture(Window::getSize(), GL_NEAREST, GL_CLAMP_TO_EDGE);
                stencilBuffer->disableDraw();
            }

            virtual void render() override {
                auto fbo = Library::getFBO("selectable");
                fbo->bind();
                CHECK_GL(glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
                CHECK_GL(glClearStencil(0));
                CHECK_GL(glViewport(0, 0, Window::getSize().x, Window::getSize().y));

                CHECK_GL(glEnable(GL_STENCIL_TEST));
                CHECK_GL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
                bind();

                /* Load PV */
                auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>();
                NEO_ASSERT(camera, "No main camera exists");
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("V", camera->get<CameraComponent>()->getView());

                for (auto& renderable : Engine::getComponentTuples<renderable::SelectableComponent, MeshComponent, SpatialComponent>()) {
                    uint32_t stencilID = renderable->get<renderable::SelectableComponent>()->mID;
                    CHECK_GL(glStencilFunc(GL_ALWAYS, static_cast<GLuint>(stencilID), 0));
                    loadUniform("componentID", static_cast<int>(stencilID));
                    loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());

                    /* DRAW */
                    renderable->get<MeshComponent>()->mMesh.draw();
                }

                unbind();
            }
        };

}
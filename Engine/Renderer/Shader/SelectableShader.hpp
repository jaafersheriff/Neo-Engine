#pragma once

#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectableComponent.hpp"
#include "ECS/Component/SelectingComponent/SelectedComponent.hpp"
#include "ECS/Component/RenderableComponent/MeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

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
            Framebuffer* stencilBuffer = Library::createFBO("selectable");
            stencilBuffer->attachStencilTexture({1, 1}, GL_NEAREST, GL_CLAMP_TO_EDGE);
            stencilBuffer->disableDraw();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message& msg, ECS& ecs) {
                NEO_UNUSED(ecs);
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage&>(msg)).mFrameSize;
                Library::getFBO("selectable")->resize(frameSize);
            });

        }

        virtual void render(const ECS& ecs) override {
            mFrameCount++;

            auto fbo = Library::getFBO("selectable");
            fbo->bind();
            NEO_ASSERT(fbo->mTextures.size() > 0, "Selectable render target never initialized");

            // Read pixels from last frame before clearing the buffer
            if (auto mouse = ecs.getSingleComponent<MouseComponent>()) {
                if (mouse->mFrameMouse.isDown(GLFW_MOUSE_BUTTON_1) && mFrameCount >= 5) {
                    MICROPROFILE_SCOPEI("Selectable Shader", "ReadPixels", MP_AUTO);
                    MICROPROFILE_SCOPEGPUI("Selectable Shader - ReadPixels", MP_AUTO);
                    uint8_t buffer[4];
                    CHECK_GL(glReadPixels(static_cast<GLint>(mouse->mFrameMouse.getPos().x), static_cast<int>(fbo->mTextures[0]->mHeight - mouse->mFrameMouse.getPos().y), 1, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, buffer));
                    uint8_t id = buffer[0];
                    if (id != mSelectedID) {
                        mSelectedID = id;
                        Messenger::sendMessage<ComponentSelectedMessage>(nullptr, mSelectedID);
                    }

                    mFrameCount = 0;
                }
            }

            CHECK_GL(glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
            CHECK_GL(glClearStencil(0));
            CHECK_GL(glViewport(0, 0, fbo->mTextures[0]->mWidth, fbo->mTextures[0]->mHeight));

            CHECK_GL(glEnable(GL_STENCIL_TEST));
            CHECK_GL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
            bind();

            /* Load PV */
            auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
            NEO_ASSERT(camera, "No main camera exists");
            loadUniform("P", camera->get<CameraComponent>()->getProj());
            loadUniform("V", camera->get<CameraComponent>()->getView());

            for (const auto& renderable : ecs.getComponentTuples<SelectableComponent, MeshComponent, SpatialComponent>()) {
                uint32_t stencilID = renderable->get<SelectableComponent>()->mID;
                CHECK_GL(glStencilFunc(GL_ALWAYS, static_cast<GLuint>(stencilID), 0));
                loadUniform("componentID", static_cast<int>(stencilID));
                loadUniform("M", renderable->get<SpatialComponent>()->getModelMatrix());

                /* DRAW */
                renderable->get<MeshComponent>()->mMesh.draw();
            }

            unbind();
        }

        private:
            uint8_t mSelectedID = 0;
            uint8_t mFrameCount = 0;
    };
}